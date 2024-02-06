/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Layer/FontManager.hpp"
#include "Layer/SkiaFontManagerProxy.hpp"
#include <core/SkStream.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkRefCnt.h>
#include <include/core/SkString.h>
#include <include/core/SkTypes.h>
#include <include/private/base/SkTArray.h>
#include <src/ports/SkFontHost_FreeType_common.h>
#include <src/core/SkOSFile.h>
#include <src/utils/SkOSPath.h>

#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

#include <optional>
#include <unordered_map>
#include <map>
#include <fstream>
#include <filesystem>
// NOLINTBEGIN
namespace fs = std::filesystem;

class SkData;
class SkFontDescriptor;
class SkStreamAsset;
class SkTypeface;

/** The base SkTypeface implementation for the custom font manager. */
class SkTypeface_VGG : public SkTypeface_FreeType
{
public:
  SkTypeface_VGG(
    const SkFontStyle& style,
    bool               isFixedPitch,
    bool               sysFont,
    const SkString     familyName,
    int                index);
  bool isSysFont() const;

protected:
  void onGetFamilyName(SkString* familyName) const override;
  void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;
  int  getIndex() const;

private:
  const bool     fIsSysFont;
  const SkString fFamilyName;
  const int      fIndex;

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
  sk_sp<SkTypeface>              onMakeClone(const SkFontArguments& args) const override;
  std::unique_ptr<SkFontData>    onMakeFontData() const override;

private:
  using INHERITED = SkTypeface_VGG;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_VGG_File : public SkTypeface_VGG
{
public:
  SkTypeface_VGG_File(
    const SkFontStyle& style,
    bool               isFixedPitch,
    bool               sysFont,
    const SkString     familyName,
    const char         path[],
    int                index);

protected:
  std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
  sk_sp<SkTypeface>              onMakeClone(const SkFontArguments& args) const override;
  std::unique_ptr<SkFontData>    onMakeFontData() const override;

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
  void              appendTypeface(sk_sp<SkTypeface> typeface);
  int               count() override;
  void              getStyle(int index, SkFontStyle* style, SkString* name) override;
  sk_sp<SkTypeface> createTypeface(int index) override;
  sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override;
  SkString          getFamilyName();

private:
  skia_private::TArray<sk_sp<SkTypeface>> fStyles;
  SkString                                fFamilyName;

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
  explicit SkFontMgrVGG(std::unique_ptr<SystemFontLoader> loader);
  explicit SkFontMgrVGG() = default;

  std::optional<std::pair<SkString, float>> fuzzyMatchFontFamilyName(
    const std::string& fontName) const;

  bool addFont(const fs::path& path);
  bool addFont(
    const uint8_t* data,
    size_t         length,
    const char*    defaultRealName,
    bool           copyData = true);

  void setFallbackFonts(std::vector<std::string> fallbacks)
  {
    m_fallbackFonts = std::move(fallbacks);
  }

  const std::vector<std::string>& fallbackFonts() const
  {
    return m_fallbackFonts;
  }

  void setFallbackEmojiFonts(std::vector<std::string> fallbacks)
  {
    m_fallbackEmojiFonts = std::move(fallbacks);
  }

  const std::vector<std::string>& fallbackEmojiFonts() const
  {
    return m_fallbackEmojiFonts;
  }

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
  std::unique_ptr<SystemFontLoader> m_loader;
  int                               onCountFamilies() const override;
  void                              onGetFamilyName(int index, SkString* familyName) const override;
  sk_sp<SkFontStyleSet>             onCreateStyleSet(int index) const override;
  sk_sp<SkFontStyleSet>             onMatchFamily(const char familyName[]) const override;
  sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[], const SkFontStyle& fontStyle)
    const override;
  sk_sp<SkTypeface> onMatchFamilyStyleCharacter(
    const char familyName[],
    const SkFontStyle&,
    const char* bcp47[],
    int         bcp47Count,
    SkUnichar   character) const override;
  sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
  sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int ttcIndex)
    const override;
  sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>, const SkFontArguments&)
    const override;
  sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
  sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override;

private:
  Families                     fFamilies;
  sk_sp<SkFontStyleSet>        fDefaultFamily;
  std::vector<std::string>     m_fallbackFonts;
  std::vector<std::string>     m_fallbackEmojiFonts;
  SkTypeface_FreeType::Scanner fScanner;
};

class VGGFontLoader : public SkFontMgrVGG::SystemFontLoader
{
public:
  VGGFontLoader(const char* d)
    : dir(SkString(d))
  {
  }

  VGGFontLoader(const std::vector<fs::path>& paths)
    : dir(paths)
  {
  }

  using TypefaceCreator = std::function<sk_sp<SkTypeface>(
    int                variationFaceIndex,
    const SkFontStyle& style,
    const SkString&    familyName,
    bool               isFixedPitch)>;

  void loadSystemFonts(
    const SkTypeface_FreeType::Scanner& scanner,
    SkFontMgrVGG::Families*             families) const override;

  static bool loadFontFromData(
    const SkTypeface_FreeType::Scanner& scanner,
    std::unique_ptr<SkMemoryStream>     stream,
    int                                 index,
    SkFontMgrVGG::Families*             families,
    const char*                         defaultRealName);

  static void loadDirectoryFonts(
    const SkTypeface_FreeType::Scanner& scanner,
    const SkString&                     directory,
    const char*                         suffix,
    SkFontMgrVGG::Families*             families);

private:
  static SkFontStyleSet_VGG* find_family(SkFontMgrVGG::Families& families, const char familyName[])
  {
    if (auto it = families.lookUp.find(familyName); it != families.lookUp.end())
    {
      return families.fFamilies[it->second].get();
    }
    return nullptr;
  }

  static bool appendTypeface(
    const SkTypeface_FreeType::Scanner& scanner,
    SkStreamAsset*                      stream,
    SkFontMgrVGG::Families*             families,
    TypefaceCreator                     creator);

  std::variant<SkString, std::vector<fs::path>> dir;
};

inline SK_API sk_sp<SkFontMgrVGG> VGGFontDirectory(const char* dir)
{
  return sk_make_sp<SkFontMgrVGG>(std::make_unique<VGGFontLoader>(dir));
}

inline SK_API sk_sp<SkFontMgrVGG> VGGFontDirectory(const std::vector<fs::path>& dir)
{
  return sk_make_sp<SkFontMgrVGG>(std::make_unique<VGGFontLoader>(dir));
}

class VGGFontCollection : public skia::textlayout::FontCollection
{
public:
  VGGFontCollection(sk_sp<SkFontMgrVGG> fontMgr)
  {
    std::vector<SkString> defFonts;
    if (fontMgr)
    {
      for (const auto& f : fontMgr->fallbackFonts())
      {
        defFonts.push_back(SkString(f));
      }
      for (const auto& f : fontMgr->fallbackEmojiFonts())
      {
        defFonts.push_back(SkString(f));
      }
    }
    this->setAssetFontManager(fontMgr);
    this->setDefaultFontManager(fontMgr, defFonts);
    this->enableFontFallback();
    this->defaultFallback();
  }

  static sk_sp<VGGFontCollection> GlobalFontCollection()
  {
    auto skmgr =
      VGG::layer::SkiaFontManagerProxy(VGG::layer::FontManager::getFontMananger()).skFontMgr();
    static sk_sp<VGGFontCollection> g_fc =
      sk_make_sp<VGGFontCollection>(sk_ref_sp<SkFontMgrVGG>(skmgr));
    return g_fc;
  }
};
// NOLINTEND
