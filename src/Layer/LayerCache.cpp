#include "LayerCache.h"
#include <include/core/SkImage.h>

namespace VGG::layer
{
// NOLINTBEGIN
BlenderCache* GlobalBlenderCache()
{
  static BlenderCache s_blenderCache(40);
  return &s_blenderCache;
}

EffectCache* GlobalEffectCache()
{
  static EffectCache s_blenderCache(40);
  return &s_blenderCache;
}

ImageCache* GlobalImageCache()
{
  static ImageCache s_imageCache(40);
  return &s_imageCache;
}

sk_sp<SkImage> loadImage(const std::string& imageGUID, const ResourceRepo& repo)
{
  sk_sp<SkImage> image;
  if (imageGUID.empty())
    return image;
  std::string guid = imageGUID;
  if (auto pos = guid.find("./"); pos != std::string::npos && pos == 0)
  {
    // remove current dir notation
    guid = guid.substr(2);
  }
  auto imageCache = GlobalImageCache();
  if (auto it = imageCache->find(guid); it)
  {
    image = *it;
  }
  else
  {
    auto repo = Scene::getResRepo();
    if (auto it = repo.find(guid); it != repo.end())
    {
      auto data = SkData::MakeWithCopy(it->second.data(), it->second.size());
      if (!data)
      {
        WARN("Make SkData failed");
        return image;
      }
      sk_sp<SkImage> skImage = SkImages::DeferredFromEncodedData(data);
      if (!skImage)
      {
        WARN("Make SkImage failed.");
        return image;
      }
      imageCache->insert(guid, skImage);
      image = skImage;
    }
    else
    {
      WARN("Cannot find %s from resources repository", guid.c_str());
    }
  }
  return image;
}
// NOLINTEND

} // namespace VGG::layer
