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
#include "Layer/Config.hpp"

#include <include/core/SkPath.h>

namespace VGG
{

// TODO:: More features added later
class VGG_EXPORTS Mask
{
public:
  SkPath outlineMask;
  Mask(){};
  void addMask(const Mask& mask)
  {
    outlineMask.addPath(mask.outlineMask);
  }

  Mask& operator=(Mask&&) noexcept = default;
  Mask(Mask&&) noexcept = default;
  Mask& operator=(const Mask&) = default;
  Mask(const Mask&) = default;
};
}; // namespace VGG
