#pragma once

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include <unordered_map>
#include <map>
#include <fstream>
#include <filesystem>
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"

namespace fs = std::filesystem;

class SkData;
class SkFontDescriptor;
class SkStreamAsset;
class SkTypeface;

/** The base SkTypeface implementation for the custom font manager. */
class SkTypeface_VGG : public SkTypeface_FreeType
{
public:
  SkTypeface_VGG(const SkFontStyle& style,
                 bool isFixedPitch,
                 bool sysFont,
                 const SkString familyName,
                 int index);
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
class SkTypeface_VGG_Empty : public SkTypeface_VGG
{
public:
  SkTypeface_VGG_Empty();

protected:
  std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;
  sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
  std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
  using INHERITED = SkTypeface_VGG;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_VGG_File : public SkTypeface_VGG
{
public:
  SkTypeface_VGG_File(const SkFontStyle& style,
                      bool isFixedPitch,
                      bool sysFont,
                      const SkString familyName,
                      const char path[],
                      int index);

protected:
  std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
  sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
  std::unique_ptr<SkFontData> onMakeFontData() const override;

private:
  SkString fPath;

  using INHERITED = SkTypeface_VGG;
};

///////////////////////////////////////////////////////////////////////////////

/**
 *  SkFontStyleSet_VGG
 *
 *  This class is used by SkFontMgrVGG to hold SkTypeface_VGG families.
 */
class SkFontStyleSet_VGG : public SkFontStyleSet
{
public:
  explicit SkFontStyleSet_VGG(const SkString familyName);

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

  friend class SkFontMgrVGG;
};

/**
 *  SkFontMgrVGG
 *
 *  This class is essentially a collection of SkFontStyleSet_VGG,
 *  one SkFontStyleSet_VGG for each family. This class may be modified
 *  to load fonts from any source by changing the initialization.
 */
class SkFontMgrVGG : public SkFontMgr
{
public:
  struct Families
  {
    // typedef skia_private::TArray<sk_sp<SkFontStyleSet_VGG>> Families;
    std::unordered_map<std::string, int> lookUp;
    using DataProxy = skia_private::TArray<sk_sp<SkFontStyleSet_VGG>>;
    using T = sk_sp<SkFontStyleSet_VGG>;
    DataProxy fFamilies;

    bool empty() const
    {
      return fFamilies.empty();
    }

    size_t size() const
    {
      return fFamilies.size();
    }
    T& push_back()
    {
      return fFamilies.push_back();
    }
    //
    const auto& operator[](int i) const
    {
      return fFamilies[i];
    }

    auto& operator[](int i)
    {
      return fFamilies[i];
    }
  };
  class SystemFontLoader
  {
  public:
    virtual ~SystemFontLoader()
    {
    }
    virtual void loadSystemFonts(const SkTypeface_FreeType::Scanner&, Families*) const = 0;
  };
  explicit SkFontMgrVGG(const SystemFontLoader& loader);

  void cacheFont(const char* familyName, int ttcIndex);

  void saveFontInfo(const fs::path& path)
  {
    std::ofstream ofs(path);
    if (ofs.is_open())
    {
      std::map<std::string, int> sort(fFamilies.lookUp.begin(), fFamilies.lookUp.end());
      for (const auto& typeface : sort)
      {
        ofs << typeface.first << std::endl;
      }
    }
  }

protected:
  int onCountFamilies() const override;
  void onGetFamilyName(int index, SkString* familyName) const override;
  sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
  sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
  sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                       const SkFontStyle& fontStyle) const override;
  sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                const SkFontStyle&,
                                                const char* bcp47[],
                                                int bcp47Count,
                                                SkUnichar character) const override;
  sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
  sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                          int ttcIndex) const override;
  sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                         const SkFontArguments&) const override;
  sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
  sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override;

private:
  Families fFamilies;
  sk_sp<SkFontStyleSet> fDefaultFamily;
  SkTypeface_FreeType::Scanner fScanner;
};

class VGGFontLoader : public SkFontMgrVGG::SystemFontLoader
{
public:
  VGGFontLoader(const char* dir)
    : fBaseDirectory(dir)
  {
  }

  void loadSystemFonts(const SkTypeface_FreeType::Scanner& scanner,
                       SkFontMgrVGG::Families* families) const override
  {
    load_directory_fonts(scanner, fBaseDirectory, ".ttf", families);
    load_directory_fonts(scanner, fBaseDirectory, ".ttc", families);
    load_directory_fonts(scanner, fBaseDirectory, ".otf", families);
    load_directory_fonts(scanner, fBaseDirectory, ".pfb", families);

    if (families->empty())
    {
      SkFontStyleSet_VGG* family = new SkFontStyleSet_VGG(SkString());
      families->push_back().reset(family);
      family->appendTypeface(sk_make_sp<SkTypeface_VGG_Empty>());
    }
  }

private:
  static SkFontStyleSet_VGG* find_family(SkFontMgrVGG::Families& families, const char familyName[])
  {
    // for (int i = 0; i < families.size(); ++i)
    // {
    //   if (families[i]->getFamilyName().equals(familyName))
    //   {
    //     return families[i].get();
    //   }
    // }
    if (auto it = families.lookUp.find(familyName); it != families.lookUp.end())
    {
      return families.fFamilies[it->second].get();
    }
    return nullptr;
  }

  static void load_directory_fonts(const SkTypeface_FreeType::Scanner& scanner,
                                   const SkString& directory,
                                   const char* suffix,
                                   SkFontMgrVGG::Families* families)
  {
    SkOSFile::Iter iter(directory.c_str(), suffix);
    SkString name;

    while (iter.next(&name, false))
    {
      SkString filename(SkOSPath::Join(directory.c_str(), name.c_str()));
      std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(filename.c_str());
      if (!stream)
      {
        // SkDebugf("---- failed to open <%s>\n", filename.c_str());
        continue;
      }

      int numFaces;
      if (!scanner.recognizedFont(stream.get(), &numFaces))
      {
        // SkDebugf("---- failed to open <%s> as a font\n", filename.c_str());
        continue;
      }

      for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex)
      {
        bool isFixedPitch;
        SkString realname;
        SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
        if (!scanner.scanFont(stream.get(), faceIndex, &realname, &style, &isFixedPitch, nullptr))
        {
          // SkDebugf("---- failed to open <%s> <%d> as a font\n",
          //          filename.c_str(), faceIndex);
          continue;
        }

        SkFontStyleSet_VGG* addTo = find_family(*families, realname.c_str());
        if (nullptr == addTo)
        {
          addTo = new SkFontStyleSet_VGG(realname);
          families->push_back().reset(addTo);
          families->lookUp[std::string(realname.c_str())] = families->size() - 1;
        }
        addTo->appendTypeface(sk_make_sp<SkTypeface_VGG_File>(style,
                                                              isFixedPitch,
                                                              true,
                                                              realname,
                                                              filename.c_str(),
                                                              faceIndex));
      }
    }

    SkOSFile::Iter dirIter(directory.c_str());
    while (dirIter.next(&name, true))
    {
      if (name.startsWith("."))
      {
        continue;
      }
      SkString dirname(SkOSPath::Join(directory.c_str(), name.c_str()));
      load_directory_fonts(scanner, dirname, suffix, families);
    }
  }

  SkString fBaseDirectory;
};

inline SK_API sk_sp<SkFontMgrVGG> VGGFontDirectory(const char* dir)
{
  return sk_make_sp<SkFontMgrVGG>(VGGFontLoader(dir));
}
