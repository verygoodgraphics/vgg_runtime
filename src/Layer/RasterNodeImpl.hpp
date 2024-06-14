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
#include "Layer/Core/RasterNode.hpp"
#include "Layer/RasterManager.hpp"

namespace VGG::layer
{

class TileTask;

class RasterNodeImpl : public RasterNode
{
public:
  RasterNodeImpl(
    VRefCnt*                       cnt,
    RasterManager::RasterExecutor* executor,
    Ref<Viewport>                  viewport,
    Ref<ZoomerNode>                zoomer,
    Ref<RenderNode>                child);

  VGG_CLASS_MAKE(RasterNodeImpl);

  void raster(const std::vector<Bounds>& bounds) override;
  void render(Renderer* renderer) override;

#ifdef VGG_LAYER_DEBUG
  void debug(Renderer* render) override;
#endif

  Bounds effectBounds() const override
  {
    return Bounds();
  }

protected:
  Bounds onRevalidate(Revalidation* inv, const glm::mat3& ctm) override;

private:
  std::unique_ptr<RasterManager> m_rasterMananger;
  glm::mat3                      m_prevMatrix;
  uint32_t                       m_pictureUniqueID = -1;
};
} // namespace VGG::layer
