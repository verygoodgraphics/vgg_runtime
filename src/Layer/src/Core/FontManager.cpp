#include <Core/FontManager.h>
namespace VGG
{

ResourceFontCollection* getDefaultFontCollection()
{
  static sk_sp<ResourceFontCollection> fontCollection =
    sk_make_sp<ResourceFontCollection>(std::vector{ fs::path("/usr/share/fonts/TTF") });
  return fontCollection.get();
}

} // namespace VGG
