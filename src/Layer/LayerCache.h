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
#pragma once

#include "LRUCache.hpp"
#include "Layer/Memory/Ref.hpp"
#include <core/SkBlender.h>
#include <core/SkImage.h>
#include <effects/SkRuntimeEffect.h>

class SkCodec;

namespace VGG::layer
{

class PaintNode;

using BlenderCacheKey = const char*;
using EffectCacheKey = const char*;
using BlenderCache = LRUCache<BlenderCacheKey, sk_sp<SkBlender>>;
using EffectCache = LRUCache<EffectCacheKey, sk_sp<SkRuntimeEffect>>;
using ImageCacheKey = std::string;
using ImageStackCache =
  LRUCache<ImageCacheKey, std::pair<std::unique_ptr<SkCodec>, std::vector<sk_sp<SkImage>>>>;

using MaskMap = std::unordered_map<std::string, WeakRef<PaintNode>>;

BlenderCache*    getGlobalBlenderCache();
EffectCache*     getGlobalEffectCache();
ImageStackCache* getGlobalImageStackCache();

std::pair<sk_sp<SkImage>, int> loadImageFromStack(const std::string& imageGUID, int i);

MaskMap* getMaskMap();
void     updateMaskMap(PaintNode* p);

} // namespace VGG::layer
