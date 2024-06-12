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
#include "Layer/Core/VBounds.hpp"
#include <core/SkPicture.h>
#include <core/SkSurface.h>
namespace VGG::layer
{

class RasterManager;

class TileTask
{
public:
  TileTask(
    RasterManager*   mgr,
    int              index,
    sk_sp<SkPicture> picture,
    const Boundsi&   damageBounds,
    sk_sp<SkSurface> surf = nullptr)
    : damageBounds(damageBounds)
    , picture(std::move(picture))
    , surf(std::move(surf))
    , mgr(mgr)
    , index(-1)
  {
  }
  Boundsi          damageBounds;
  sk_sp<SkPicture> picture;
  sk_sp<SkSurface> surf; // optional, null indicates this is a new surface
  RasterManager*   mgr;
  int              index;
};

class RasterTask
{
public:
  struct Where
  {
    struct
    {
      int x, y;
    } dst;
    Boundsi src;
  };
  SkColor            bgColor;
  int                width, height;
  std::vector<Where> where;
  sk_sp<SkPicture>   picture;
  sk_sp<SkSurface>   surf; // optional, null indicates a new surface should be created, and the
                           // surface would be (width, height)
};

} // namespace VGG::layer
