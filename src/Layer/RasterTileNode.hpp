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
#include "Layer/Core/RasterNode.hpp"
#include "Layer/RasterManager.hpp"

#include <vector>

class GrRecordingContext;

namespace VGG::layer
{

class Viewport;
class ZoomerNode;
class RenderNode;

class TileRasterNode : public RasterNode
{
public:
  TileRasterNode(
    VRefCnt*                       cnt,
    GrRecordingContext*            device,
    RasterManager::RasterExecutor* executor,
    Ref<Viewport>                  viewport,
    Ref<ZoomerNode>                zoomer,
    Ref<RenderNode>                child);

  VGG_CLASS_MAKE(TileRasterNode);

  void raster(const std::vector<Bounds>& bounds) override
  {
  }

  void render(Renderer* renderer) override;

#ifdef VGG_LAYER_DEBUG
  void debug(Renderer* render) override;
#endif

  Bounds effectBounds() const override
  {
    return Bounds();
  }

  Bounds onRevalidate(Revalidation* inv, const glm::mat3& mat) override;

private:
  glm::mat3 m_matrix;

  std::unique_ptr<Rasterizer>          m_raster;
  std::vector<layer::Rasterizer::Tile> m_rasterTiles;
  SkMatrix                             m_rasterMatrix;
  int64_t                              m_cacheUniqueID{ -1 };
};
} // namespace VGG::layer
