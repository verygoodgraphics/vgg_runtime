#pragma once
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>
#include <filesystem>

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

public:
  FontManager(const FontManager& s) = delete;
  void operator=(const FontManager&) = delete;
  void initFontManager(const std::vector<fs::path>& fontDirs);

  bool hasInit()
  {
    return m_init;
  }

  void registerFontDirectory(const fs::path& dir);

  void registerFontFile(const std::string& fontName);

  sk_sp<ResourceFontCollection> defaultFontCollection();

  static FontManager& instance()
  {
    static FontManager F;
    return F;
  }

private:
  FontManager();
};

} // namespace VGG
