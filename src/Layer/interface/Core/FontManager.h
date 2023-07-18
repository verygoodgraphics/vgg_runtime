#pragma once
#include <core/SkFontMgr.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>
#include "Layer/include/SkiaBackend/SkFontMgrVGG.h"
#include "include/ports/SkFontMgr_directory.h"
#include <unordered_map>

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
    : fFontProvider(VGGFontDirectory(fontDir.string().c_str()))
  {
    if (testOnly)
    {
      this->setTestFontManager(std::move(fFontProvider));
    }
    else
    {
      this->setAssetFontManager(std::move(fFontProvider));
    }
    this->disableFontFallback();
  }

private:
  sk_sp<SkFontMgr> fFontProvider;
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
