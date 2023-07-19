#pragma once
#include <core/SkFontMgr.h>
#include <core/SkString.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>
#include "Layer/include/SkiaBackend/SkFontMgrVGG.h"
#include "include/ports/SkFontMgr_directory.h"
#include <string>
#include <unordered_map>
#include <ConfigMananger.h>

#include <filesystem>
#include <iostream>

namespace VGG
{
using namespace skia::textlayout;
namespace fs = std::filesystem;

class VGGFontCollection : public FontCollection
{
public:
  VGGFontCollection(const fs::path& fontDir, const std::vector<std::string>& fallbackFonts)
  {
    auto fFontProvider = VGGFontDirectory(fontDir.string().c_str());
    fFontProvider->saveFontInfo("FontName.txt");
    std::vector<SkString> defFonts;
    for (const auto& f : fallbackFonts)
    {
      defFonts.push_back(SkString(f));
    }
    this->setAssetFontManager(fFontProvider);
    this->setDefaultFontManager(fFontProvider, defFonts);
    this->enableFontFallback();
  }
};

class FontManager
{
  sk_sp<VGGFontCollection> m_defaultFontCollection;
  std::unordered_map<std::string, sk_sp<VGGFontCollection>> fontResourceCache;
  std::unordered_map<std::string, std::vector<fs::path>> fontFilenameCache;
  std::string defaultFontCollectionKey;

public:
  FontManager(const FontManager& s) = delete;
  void operator=(const FontManager&) = delete;
  sk_sp<VGGFontCollection> createOrGetFontCollection(const std::string& key,
                                                     const fs::path& fontDirs,
                                                     const std::vector<std::string>& fallbackFonts);

  sk_sp<VGGFontCollection> fontCollection(const std::string& key);

  sk_sp<FontCollection> defaultFontCollection()
  {
    return m_defaultFontCollection;
  }

  static FontManager& instance()
  {
    static FontManager F;
    return F;
  }

private:
  FontManager();
};

} // namespace VGG
