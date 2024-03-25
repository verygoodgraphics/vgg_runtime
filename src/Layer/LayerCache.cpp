#include "LayerCache.h"
#include <include/core/SkImage.h>

namespace VGG::layer
{
BlenderCache* getGlobalBlenderCache()
{
  static BlenderCache s_blenderCache(40);
  return &s_blenderCache;
}

EffectCache* getGlobalEffectCache()
{
  static EffectCache s_blenderCache(40);
  return &s_blenderCache;
}

ImageCache* getGlobalImageCache()
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
  auto imageCache = getGlobalImageCache();
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

MaskMap* getMaskMap()
{
  static MaskMap s_maskMap;
  return &s_maskMap;
}

namespace
{
void updateMaskMapInternal(PaintNode* p)
{
  if (!p)
    return;
  auto objects = getMaskMap();
  if (p->maskType() != MT_NONE)
  {
    if (auto it = objects->find(p->guid()); it == objects->end())
    {
      (*objects)[p->guid()] = p; // type of all children of paintnode must be paintnode
    }
  }
  for (auto it = p->begin(); it != p->end(); ++it)
  {
    updateMaskMapInternal(static_cast<layer::PaintNode*>(it->get()));
  }
}
} // namespace

void updateMaskMap(PaintNode* p)
{
  getMaskMap()->clear();
  updateMaskMapInternal(p);
}

} // namespace VGG::layer
