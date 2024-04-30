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
#include "Layer/Core/RasterCache.hpp"
#include "Layer/LRUCache.hpp"
#include "Layer/VSkia.hpp"
#include "Utility/HelperMacro.hpp"
#include "Utility/Log.hpp"

#include <core/SkMatrix.h>

class SkSurface;

class Zoomer;
namespace VGG::layer
{
class RasterCacheTile__pImpl;
class RasterCacheTile : public Rasterizer
{
  VGG_DECL_IMPL(RasterCacheTile);

public:
  RasterCacheTile();
  void purge() override;
  ~RasterCacheTile();

protected:
  std::tuple<uint32_t, std::vector<Tile>, SkMatrix> onRevalidateRaster(
    uint32_t             reason,
    GrRecordingContext*  context,
    int                  lod,
    const SkRect&        clipRect,
    const RasterContext& rasterContext,
    void*                userData) override;
};

} // namespace VGG::layer
