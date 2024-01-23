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
// #include "Layer/Core/RasterCache.hpp"
// #include "Layer/VSkia.hpp"
// #include "Layer/Zoomer.hpp"
// #include "Utility/Log.hpp"
//
// #include "core/SkCanvas.h"
// #include "core/SkImage.h"
// #include "core/SkPicture.h"
// #include <core/SkSurface.h>
// #include <gpu/ganesh/SkSurfaceGanesh.h>
// #include <gpu/GpuTypes.h>

// class Zoomer;
// namespace VGG::layer
// {
// class RasterCacheDefault : public Rasterizer
// {
//   std::vector<Tile> m_caches;
//
//   std::string printReason(uint32_t r)
//   {
//     std::string res;
//     if (r == 0)
//       return "";
//     if (r & ZOOM_TRANSLATION)
//     {
//       res += " ZOOM_TRANSLATION";
//     }
//     if (r & ZOOM_SCALE)
//     {
//       res += " ZOOM_SCALE";
//     }
//     if (r & VIEWPORT)
//     {
//       res += " VIEWPORT";
//     }
//     if (r & CONTENT)
//     {
//       res += " CONTENT";
//     }
//     return res;
//   }
//
// public:
//   RasterCacheDefault() = default;
//
//   void onQueryTile(Tile** tiles, int* count, SkMatrix* transform) override
//   {
//     *tiles = m_caches.data();
//     *count = m_caches.size();
//     (*transform)[SkMatrix::kMScaleX] = 1;
//     (*transform)[SkMatrix::kMScaleY] = 1;
//   }
//
//   uint32_t onRaster(
//     uint32_t            reason,
//     GrRecordingContext* context,
//     const SkMatrix*     transform,
//     const SkRect&       viewport,
//     SkPicture*          pic,
//     const SkRect&       bound,
//     const SkMatrix&     mat,
//     void*               userData) override
//   {
//     auto str = printReason(reason);
//     DEBUG("recache reason: %s", str.c_str());
//     if ((reason & ZOOM_TRANSLATION) && !(reason & ZOOM_SCALE))
//     {
//       DEBUG("ignore zoom translation");
//       return reason;
//     }
//     // if (reason & ZOOM_SCALE)
//     // {
//     //   DEBUG("current zoom scale: %f", zoomScale);
//     //   auto ignoreScale = [](float prev, float current) -> bool
//     //   { return std::abs(prev - current) < 0.4; };
//     //   if (m_scale && ignoreScale(*m_scale, zoomScale))
//     //   {
//     //     DEBUG("ignore zoom scale");
//     //     return reason;
//     //   }
//     // }
//     auto        skm = *transform * mat;
//     float       scaleX = skm.getScaleX();
//     float       scaleY = skm.getScaleY();
//     SkImageInfo info = SkImageInfo::MakeN32Premul(bound.width() * scaleX, bound.height() *
//     scaleY); if (auto surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
//     surface)
//     {
//       DEBUG("Raster Image Size: [%f, %f]", scaleX, scaleY);
//       auto canvas = surface->getCanvas();
//       canvas->scale(scaleX, scaleY);
//       canvas->concat(mat);
//       canvas->drawPicture(pic);
//       m_caches = { { surface->makeImageSnapshot(),
//                      SkRect::MakeXYWH(0, 0, info.width(), info.height()) } };
//     }
//     else
//     {
//       // Maybe it's too large, so just cache as viewport size info =
//       SkImageInfo::MakeN32Premul(viewport.width(), viewport.height());
//       if (auto surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info); surface)
//       {
//         auto canvas = surface->getCanvas();
//         canvas->clipRect(viewport);
//         canvas->setMatrix(*transform);
//         canvas->drawPicture(pic);
//         m_caches = { { surface->makeImageSnapshot(),
//                        SkRect::MakeXYWH(0, 0, viewport.width(), viewport.height()) } };
//       }
//       else
//       {
//         DEBUG("Raster cache failed");
//         return 0;
//       }
//     }
//     return reason;
//   }
// };
//
// } // namespace VGG::layer
