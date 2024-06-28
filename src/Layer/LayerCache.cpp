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
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/ResourceManager.hpp"
#include <include/codec/SkCodec.h>
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

ImageStackCache* getGlobalImageStackCache()
{
  static ImageStackCache s_imageStackCache(40);
  return &s_imageStackCache;
}

ResourcesCache* getGlobalResourcesCache()
{
  static ResourcesCache s_blobCache(40);
  return &s_blobCache;
}

sk_sp<SkData> loadBlob(const std::string& guid)
{
  if (auto cache = getGlobalResourcesCache(); cache)
  {
    if (auto it = cache->find(guid); it)
    {
      return *it;
    }
    else
    {
      if (auto repo = getGlobalResourceProvider(); repo)
      {
        if (auto data = repo->readData(guid); data)
        {
          cache->insert(guid, data);
          return data;
        }
        else
        {
          WARN("Cannot find %s from resources repository", guid.data());
        }
      }
    }
  }
  return nullptr;
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

std::pair<sk_sp<SkImage>, int> loadImageFromStack(const std::string& imageGUID, int i)
{
  if (imageGUID.empty())
    return { nullptr, 0 };
  sk_sp<SkImage> image;
  int            totalCount = 0;
  auto           imageCache = getGlobalImageStackCache();

  auto decode = [&imageGUID](SkCodec* codec, int i) -> sk_sp<SkImage>
  {
    SkCodec::Options options;
    options.fFrameIndex = i;
    auto [img, res] = codec->getImage(codec->getInfo(), &options);
    if (res != SkCodec::Result::kSuccess)
    {
      DEBUG("can not decode image [%d] %s", i, imageGUID.c_str());
      return nullptr;
    }
    return img;
  };

  if (auto it = imageCache->find(imageGUID); it)
  {
    if (!it->first)
      return { nullptr, 0 };
    if (!it->second[i])
    {
      it->second[i] = decode(it->first.get(), i);
    }
    image = it->second[i];
    totalCount = it->second.size();
  }
  else
  {
    if (auto repo = getGlobalResourceProvider(); repo)
    {
      if (auto data = repo->readData(imageGUID); data)
      {
        auto blob = loadBlob(imageGUID);
        if (!blob)
        {
          return { nullptr, 0 };
        }
        auto                        codec = SkCodec::MakeFromData(blob);
        std::vector<sk_sp<SkImage>> stack(codec->getFrameCount());
        if (!stack[i])
        {
          stack[i] = decode(codec.get(), i);
        }
        image = stack[i];
        totalCount = stack.size();
        imageCache->insert(imageGUID, std::pair{ std::move(codec), std::move(stack) });
      }
      else
      {
        WARN("Cannot find %s from resources repository", imageGUID.c_str());
      }
    }
  }
  return { image, totalCount };
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
    else if (auto ptr = it->second.lock(); !ptr)
    {
      objects->erase(it);
      (*objects)[p->guid()] = p;
    }
  }
  for (auto it = p->begin(); it != p->end(); ++it)
  {
    updateMaskMapInternal(it->get());
  }
}
} // namespace

void updateMaskMap(PaintNode* p)
{
  // getMaskMap()->clear();
  updateMaskMapInternal(p);
}

} // namespace VGG::layer
