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
#include "VSkia.hpp"
#include "Layer/Core/VBounds.hpp"
#include "Layer/RasterManager.hpp"
#include <core/SkPicture.h>
#include <core/SkSurface.h>

#include <gpu/GrRecordingContext.h>
#include <gpu/GpuTypes.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>

namespace VGG::layer
{

class RasterManager;

class TileTask : public RasterManager::RasterTask
{
public:
  struct Where
  {
    struct
    {
      int x, y;
    } dst;
    Bounds src;
  };
  TileTask(
    RasterManager*     mgr,
    size_t             index,
    int                tw,
    int                th,
    std::vector<Where> where,
    const glm::mat3&   matrix,
    sk_sp<SkPicture>   picture,
    sk_sp<SkSurface>   surf = nullptr)
    : RasterTask(index)
    , matrix(matrix)
    , damage(std::move(where))
    , tw(tw)
    , th(th)
    , picture(std::move(picture))
    , surf(std::move(surf))
    , mgr(mgr)
  {
  }

  SkColor            bgColor;
  glm::mat3          matrix;
  std::vector<Where> damage;
  int                tw, th;
  sk_sp<SkPicture>   picture;
  sk_sp<SkSurface>   surf; // optional, null indicates this is a new surface
  RasterManager*     mgr;

  RasterManager::RasterResult execute(GrRecordingContext* context) override
  {
    using RR = RasterManager::RasterResult;
    const int width = tw;
    const int height = th;
    if (!surf || surf->width() != width || surf->height() != height)
    {
      auto rasterSurface = [](GrRecordingContext* context, int w, int h)
      {
        const auto info = SkImageInfo::MakeN32Premul(w, h);
        return SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
      };

      surf = rasterSurface(context, width, height);
      ASSERT(surf);
      auto canvas = surf->getCanvas();
      canvas->clear(bgColor);
    }

    auto canvas = surf->getCanvas();
    for (const auto& b : damage)
    {
      canvas->save();
      canvas->translate(b.dst.x - b.src.x(), b.dst.y - b.src.y());
      auto clipRect = SkRect::MakeXYWH(b.src.x(), b.src.y(), b.src.width(), b.src.height());
      canvas->clipRect(clipRect);
      canvas->concat(toSkMatrix(matrix));
      canvas->clear(bgColor);
      picture->playback(canvas);
      canvas->restore();
    }
    return RR(mgr, std::move(surf), index());
  }
};

class SurfaceTask : public RasterManager::RasterTask
{
public:
  struct Where
  {
    struct
    {
      int x, y;
    } dst;
    Bounds src;
  };
  SkColor            bgColor;
  int                width, height;
  glm::mat3          matrix;
  std::vector<Where> where;
  sk_sp<SkPicture>   picture;
  sk_sp<SkSurface>   surf; // optional, null indicates a new surface should be created, and the
                           // surface would be (width, height)

  SurfaceTask(
    int                index,
    SkColor            bgColor,
    int                width,
    int                height,
    glm::mat3          matrix,
    std::vector<Where> where,
    sk_sp<SkPicture>   picture,
    sk_sp<SkSurface>   surf = nullptr)
    : RasterManager::RasterTask(index)
    , bgColor(bgColor)
    , width(width)
    , height(height)
    , matrix(matrix)
    , where(std::move(where))
    , picture(std::move(picture))
    , surf(std::move(surf))
  {
  }

  RasterManager::RasterResult execute(GrRecordingContext* context) override
  {
    if (!surf || surf->width() != width || surf->height() != height)
    {
      auto rasterSurface = [](GrRecordingContext* context, int w, int h)
      {
        const auto info = SkImageInfo::MakeN32Premul(w, h);
        return SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
      };

      surf = rasterSurface(context, width, height);
      ASSERT(surf);
      auto canvas = surf->getCanvas();
      canvas->clear(bgColor);
    }
    ASSERT(surf);
    auto canvas = surf->getCanvas();

    for (const auto& b : where)
    {
      const auto rasterRect = b.src.map(matrix);
      canvas->save();
      canvas->translate(b.dst.x - rasterRect.x(), b.dst.y - rasterRect.y());
      canvas->concat(toSkMatrix(matrix));
      canvas->clipRect(SkRect::MakeXYWH(b.src.x(), b.src.y(), b.src.width(), b.src.height()));
      canvas->clear(bgColor);
      picture->playback(canvas);
      canvas->restore();
    }
    return RasterManager::RasterResult(nullptr, std::move(surf), index());
  }
};

} // namespace VGG::layer
