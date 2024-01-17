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
#include "Effects.hpp"
#include "Layer/LayerCache.h"
#include <effects/SkRuntimeEffect.h>

namespace VGG::layer
{
// NOLINTBEGIN
sk_sp<SkBlender> GetOrCreateBlender(EffectCacheKey name, const char* sksl)
{
  auto cache = GlobalBlenderCache();
  auto b = cache->find(name);
  if (!b)
  {
    auto result = SkRuntimeEffect::MakeForBlender(SkString(sksl));
    if (!result.effect)
    {
      DEBUG("Runtime Effect Failed[%s]: %s", name, result.errorText.data());
      return nullptr;
    }
    auto blender = result.effect->makeBlender(nullptr);
    return *cache->insert(name, std::move(blender));
  }
  return *b;
}

sk_sp<SkRuntimeEffect> GetOrCreateEffect(EffectCacheKey key, const char* sksl)
{
  auto cache = GlobalEffectCache();
  auto b = cache->find(key);
  if (!b)
  {
    auto result = SkRuntimeEffect::MakeForColorFilter(SkString(sksl));
    if (!result.effect)
    {
      DEBUG("Runtime Effect Failed[%s]: %s", key, result.errorText.data());
      return nullptr;
    }
    return *cache->insert(key, std::move(result.effect));
  }
  return *b;
}
// NOLINTEND

} // namespace VGG::layer
