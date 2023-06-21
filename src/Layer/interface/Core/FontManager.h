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

private:
  sk_sp<TypefaceFontProvider> fFontProvider;
};

ResourceFontCollection* getDefaultFontCollection();
} // namespace VGG
