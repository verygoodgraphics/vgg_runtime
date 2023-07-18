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

class ResourceFontCollection : public FontCollection
{
public:
  ResourceFontCollection(const fs::path& fontDir, bool testOnly = false)
  {
    auto fFontProvider = VGGFontDirectory(fontDir.string().c_str());
    fFontProvider->saveFontInfo("FontName.txt");
    auto& cfg = Config::globalConfig();
    std::vector<SkString> defFonts;
    if (auto it = cfg.find("fallbackFonts"); it != cfg.end())
    {
      std::vector<std::string> fallbackFonts = *it;
      for (const auto& f : fallbackFonts)
      {
        defFonts.push_back(SkString(f));
      }
    }
    this->setAssetFontManager(fFontProvider);
    this->setDefaultFontManager(fFontProvider, defFonts);
    this->enableFontFallback();
  }
};

class FontManager
{
  sk_sp<ResourceFontCollection> m_defaultFontCollection;
  bool m_init{ false };
  std::unordered_map<std::string, sk_sp<ResourceFontCollection>> fontResourceCache;
  std::unordered_map<std::string, std::vector<fs::path>> fontFilenameCache;

public:
  FontManager(const FontManager& s) = delete;
  void operator=(const FontManager&) = delete;
  bool hasInit()
  {
    return m_init;
  }

  sk_sp<ResourceFontCollection> createOrGetFontCollection(const std::string& key,
                                                          const fs::path& fontDirs);

  sk_sp<ResourceFontCollection> fontCollection(const std::string& key);

  static FontManager& instance()
  {
    static FontManager F;
    return F;
  }

private:
  FontManager();
};

} // namespace VGG
