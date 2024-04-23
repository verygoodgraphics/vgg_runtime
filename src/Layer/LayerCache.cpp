/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "LayerCache.h"
#include "Layer/Core/ResourceManager.hpp"
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

sk_sp<SkImage> loadImage(const std::string& imageGUID)
{
  sk_sp<SkImage> image;
  if (imageGUID.empty())
    return image;
  auto imageCache = getGlobalImageCache();
  if (auto it = imageCache->find(imageGUID); it)
  {
    image = *it;
  }
  else
  {
    if (auto repo = getGlobalResourceProvider(); repo)
    {
      if (auto data = repo->readData(imageGUID); data)
      {
        sk_sp<SkImage> skImage = SkImages::DeferredFromEncodedData(data);
        if (!skImage)
        {
          WARN("Make SkImage failed.");
          return image;
        }
        imageCache->insert(imageGUID, skImage);
        image = skImage;
      }
      else
      {
        WARN("Cannot find %s from resources repository", imageGUID.c_str());
      }
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
  // getMaskMap()->clear();
  updateMaskMapInternal(p);
}

} // namespace VGG::layer
