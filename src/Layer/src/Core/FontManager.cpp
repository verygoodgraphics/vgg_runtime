#include <Core/FontManager.h>
#include <core/SkRefCnt.h>
namespace VGG
{

ResourceFontCollection* getDefaultFontCollection()
{
  static sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(
    std::vector{ fs::path("/usr/share/fonts/TTF"), fs::path("/home/ysl/Code/vgg_runtime/fonts") });
  return fontCollection.get();
}
FontManager::FontManager()
{
}
sk_sp<ResourceFontCollection> FontManager::createOrGetFontCollection(
  const std::string& key,
  const std::vector<fs::path>& fontDirs)
{
  sk_sp<ResourceFontCollection> fontCollection;
  if (auto it = fontResourceCache.find(key); it != fontResourceCache.end())
  {
    return it->second;
  }
  else
  {
    fontCollection = sk_make_sp<ResourceFontCollection>(fontDirs);
    fontResourceCache[key] = fontCollection;
    return fontCollection;
  }
}

sk_sp<ResourceFontCollection> FontManager::fontCollection(const std::string& key)
{
  sk_sp<ResourceFontCollection> fontCollection;
  if (auto it = fontResourceCache.find(key); it != fontResourceCache.end())
  {
    fontCollection = it->second;
  }
  return fontCollection;
}

} // namespace VGG
