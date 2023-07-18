#pragma once

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"

class SkData;
class SkFontDescriptor;
class SkStreamAsset;
class SkTypeface;

/** The base SkTypeface implementation for the custom font manager. */
class SkTypeface_Custom : public SkTypeface_FreeType {
public:
    SkTypeface_Custom(const SkFontStyle& style, bool isFixedPitch,
                      bool sysFont, const SkString familyName, int index);
    bool isSysFont() const;

protected:
    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;
    int getIndex() const;

private:
    const bool fIsSysFont;
    const SkString fFamilyName;
    const int fIndex;

    using INHERITED = SkTypeface_FreeType;
};

/** The empty SkTypeface implementation for the custom font manager.
 *  Used as the last resort fallback typeface.
 */
class SkTypeface_Empty : public SkTypeface_Custom {
public:
    SkTypeface_Empty() ;

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    using INHERITED = SkTypeface_Custom;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_File : public SkTypeface_Custom {
public:
    SkTypeface_File(const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                    const SkString familyName, const char path[], int index);

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
    SkString fPath;

    using INHERITED = SkTypeface_Custom;
};

///////////////////////////////////////////////////////////////////////////////

/**
 *  SkFontStyleSet_Custom
 *
 *  This class is used by SkFontMgr_Custom to hold SkTypeface_Custom families.
 */
class SkFontStyleSet_Custom : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_Custom(const SkString familyName);

    /** Should only be called during the initial build phase. */
    void appendTypeface(sk_sp<SkTypeface> typeface);
    int count() override;
    void getStyle(int index, SkFontStyle* style, SkString* name) override;
    sk_sp<SkTypeface> createTypeface(int index) override;
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override;
    SkString getFamilyName();

private:
    skia_private::TArray<sk_sp<SkTypeface>> fStyles;
    SkString fFamilyName;

    friend class SkFontMgr_Custom;
};

/**
 *  SkFontMgr_Custom
 *
 *  This class is essentially a collection of SkFontStyleSet_Custom,
 *  one SkFontStyleSet_Custom for each family. This class may be modified
 *  to load fonts from any source by changing the initialization.
 */
class SkFontMgr_Custom : public SkFontMgr {
public:
    typedef skia_private::TArray<sk_sp<SkFontStyleSet_Custom>> Families;
    class SystemFontLoader {
    public:
        virtual ~SystemFontLoader() { }
        virtual void loadSystemFonts(const SkTypeface_FreeType::Scanner&, Families*) const = 0;
    };
    explicit SkFontMgr_Custom(const SystemFontLoader& loader);

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontStyle) const override;
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                  const char* bcp47[], int bcp47Count,
                                                  SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>, const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override;

private:
    Families fFamilies;
    sk_sp<SkFontStyleSet> fDefaultFamily;
    SkTypeface_FreeType::Scanner fScanner;
};

class DirectorySystemFontLoader : public SkFontMgr_Custom::SystemFontLoader {
public:
    DirectorySystemFontLoader(const char* dir) : fBaseDirectory(dir) { }

    void loadSystemFonts(const SkTypeface_FreeType::Scanner& scanner,
                         SkFontMgr_Custom::Families* families) const override
    {
        load_directory_fonts(scanner, fBaseDirectory, ".ttf", families);
        load_directory_fonts(scanner, fBaseDirectory, ".ttc", families);
        load_directory_fonts(scanner, fBaseDirectory, ".otf", families);
        load_directory_fonts(scanner, fBaseDirectory, ".pfb", families);

        if (families->empty()) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            families->push_back().reset(family);
            family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
        }
    }

private:
    static SkFontStyleSet_Custom* find_family(SkFontMgr_Custom::Families& families,
                                              const char familyName[])
    {
       for (int i = 0; i < families.size(); ++i) {
            if (families[i]->getFamilyName().equals(familyName)) {
                return families[i].get();
            }
        }
        return nullptr;
    }

    static void load_directory_fonts(const SkTypeface_FreeType::Scanner& scanner,
                                     const SkString& directory, const char* suffix,
                                     SkFontMgr_Custom::Families* families)
    {
        SkOSFile::Iter iter(directory.c_str(), suffix);
        SkString name;

        while (iter.next(&name, false)) {
            SkString filename(SkOSPath::Join(directory.c_str(), name.c_str()));
            std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(filename.c_str());
            if (!stream) {
                // SkDebugf("---- failed to open <%s>\n", filename.c_str());
                continue;
            }

            int numFaces;
            if (!scanner.recognizedFont(stream.get(), &numFaces)) {
                // SkDebugf("---- failed to open <%s> as a font\n", filename.c_str());
                continue;
            }

            for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
                bool isFixedPitch;
                SkString realname;
                SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
                if (!scanner.scanFont(stream.get(), faceIndex,
                                      &realname, &style, &isFixedPitch, nullptr))
                {
                    // SkDebugf("---- failed to open <%s> <%d> as a font\n",
                    //          filename.c_str(), faceIndex);
                    continue;
                }

                SkFontStyleSet_Custom* addTo = find_family(*families, realname.c_str());
                if (nullptr == addTo) {
                    addTo = new SkFontStyleSet_Custom(realname);
                    families->push_back().reset(addTo);
                }
                addTo->appendTypeface(sk_make_sp<SkTypeface_File>(style, isFixedPitch, true,
                                                                  realname, filename.c_str(),
                                                                  faceIndex));
            }
        }

        SkOSFile::Iter dirIter(directory.c_str());
        while (dirIter.next(&name, true)) {
            if (name.startsWith(".")) {
                continue;
            }
            SkString dirname(SkOSPath::Join(directory.c_str(), name.c_str()));
            load_directory_fonts(scanner, dirname, suffix, families);
        }
    }

    SkString fBaseDirectory;
};

inline SK_API sk_sp<SkFontMgr> VGGFontDirectory(const char* dir) {
    return sk_make_sp<SkFontMgr_Custom>(DirectorySystemFontLoader(dir));
}

