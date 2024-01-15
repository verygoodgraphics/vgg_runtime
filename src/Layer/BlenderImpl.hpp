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

#include "Layer/LRUCache.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/LayerCache.h"
#include "Layer/SkSL.hpp"
#include <core/SkBlender.h>
#include <include/effects/SkRuntimeEffect.h>

namespace VGG::layer
{
inline sk_sp<SkBlender> GetOrCreateBlender(const char* name, const char* sksl) // NOLINT
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

inline sk_sp<SkBlender> getMaskBlender(AlphaMaskType type)
{
  switch (type)
  {
    case AM_ALPHA:
      return GetOrCreateBlender("alpha", g_alphaMaskBlender);
    case AM_LUMINOSITY:
      return GetOrCreateBlender("lumi", g_luminosityBlender);
    case AM_INVERSE_LUMINOSITY:
      return GetOrCreateBlender("invLumi", g_invLuminosityBlender);
  }
  DEBUG("No corresponding mask blender");
  return nullptr;
}

} // namespace VGG::layer
