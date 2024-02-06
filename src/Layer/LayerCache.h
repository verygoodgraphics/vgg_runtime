/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#pragma once

#include "LRUCache.hpp"
#include "Layer/Scene.hpp"
#include <core/SkBlender.h>
#include <effects/SkRuntimeEffect.h>

namespace VGG::layer
{

using BlenderCacheKey = const char*;
using EffectCacheKey = const char*;
using BlenderCache = LRUCache<BlenderCacheKey, sk_sp<SkBlender>>;
using EffectCache = LRUCache<EffectCacheKey, sk_sp<SkRuntimeEffect>>;

using ImageCacheKey = std::string;
using ImageCache = LRUCache<ImageCacheKey, sk_sp<SkImage>>;

BlenderCache*  getGlobalBlenderCache();
EffectCache*   getGlobalEffectCache();
ImageCache*    getGlobalImageCache();
sk_sp<SkImage> loadImage(const std::string& imageGUID, const ResourceRepo& repo);

} // namespace VGG::layer
