#pragma once
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>
#include <filesystem>
#include <iostream>

namespace VGG
{
using namespace skia::textlayout;
namespace fs = std::filesystem;

class ResourceFontCollection : public FontCollection
{
public:
  ResourceFontCollection(const std::vector<fs::path>& fontDirs, bool testOnly = false)
    : fFontProvider(sk_make_sp<TypefaceFontProvider>())
  {

    for (const auto& p : fontDirs)
    {
      for (const auto& ent : fs::recursive_directory_iterator(p))
      {
        auto typeface = SkTypeface::MakeFromFile(fs::path(ent).string().c_str());
        if (typeface)
        {
          fFontProvider->registerTypeface(typeface);
          // SkString name, psname;
          // typeface->getFamilyName(&name);
          // typeface->getPostScriptName(&psname);
          // std::cout << "familiy name: " << name.c_str() << " " << psname.c_str() << std::endl;
        }
      }
    }

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

  void registerFontDir(const fs::path& dir)
  {
    auto typeface = this->getFallbackManager();
  }

private:
  sk_sp<TypefaceFontProvider> fFontProvider;
};

ResourceFontCollection* getDefaultFontCollection();

class FontManager
{
  sk_sp<ResourceFontCollection> m_defaultFontCollection;
  bool m_init{ false };
  std::unordered_map<std::string, sk_sp<ResourceFontCollection>> fontResourceCache;

public:
  FontManager(const FontManager& s) = delete;
  void operator=(const FontManager&) = delete;
  bool hasInit()
  {
    return m_init;
  }

  sk_sp<ResourceFontCollection> createOrGetFontCollection(const std::string& key,
                                                          const std::vector<fs::path>& fontDirs);

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
