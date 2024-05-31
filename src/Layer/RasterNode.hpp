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
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Renderable.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Core/RasterCache.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Config.hpp"

#include "Layer/ViewportNode.hpp"

#include <gpu/GrRecordingContext.h>
#include <map>
#include <memory>

namespace VGG::layer
{
class RasterNode : public TransformEffectNode
{
public:
  RasterNode(
    VRefCnt*            cnt,
    GrRecordingContext* device,
    Ref<Viewport>       viewport,
    Ref<ZoomerNode>     zoomer,
    Ref<RenderNode>     child);

  VGG_CLASS_MAKE(RasterNode);

  void render(Renderer* renderer) override;

#ifdef VGG_LAYER_DEBUG
  void debug(Renderer* render) override;
#endif

  Bounds effectBounds() const override
  {
    return Bounds();
  }

  Bounds onRevalidate(Revalidation* inv, const glm::mat3& mat) override;

  ~RasterNode() override
  {
    unobserve(m_viewport);
  }

private:
  GrRecordingContext* m_device{ nullptr };
  Ref<Viewport>       m_viewport;
  glm::mat3           m_matrix;

  std::unique_ptr<Rasterizer>          m_raster;
  std::vector<layer::Rasterizer::Tile> m_rasterTiles;
  SkMatrix                             m_rasterMatrix;
  int64_t                              m_cacheUniqueID{ -1 };
};
} // namespace VGG::layer
