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
#include "Layer/Core/VNode.hpp"
#include "Layer/RasterManager.hpp"
#include "Layer/Renderable.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Core/RasterCache.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Layer/Config.hpp"

#include "Layer/Core/ViewportNode.hpp"

#include <map>
#include <memory>

class GrRecordingContext;

namespace VGG::layer
{
class Rasterizer;
class RasterExecutor;

class RasterNode : public TransformEffectNode
{
public:
  RasterNode(
    VRefCnt*                       cnt,
    GrRecordingContext*            device,
    RasterManager::RasterExecutor* executor,
    Ref<Viewport>                  viewport,
    Ref<ZoomerNode>                zoomer,
    Ref<RenderNode>                child,
    EDamageTrait                   trait = 0);

  virtual void raster(const std::vector<Bounds>& bounds) = 0;

  const glm::mat3& getRasterMatrix() const
  {
    ASSERT(!isInvalid());
    return m_rasterMatrix;
  }

  const glm::mat3& getLocalMatrix() const
  {
    ASSERT(!isInvalid());
    return m_localMatrix;
  }

  glm::mat3 getInversedRasterMatrix() const
  {
    ASSERT(!isInvalid());
    return glm::inverse(m_rasterMatrix); // OPTIMIZED
  }

  glm::mat3 getInversedLocalMatrix() const
  {
    ASSERT(!isInvalid());
    return glm::inverse(m_localMatrix); // OPTIMIZED
  }

  Bounds worldBoundsInRasterSpace() const
  {
    ASSERT(!isInvalid());
    ASSERT(getChild());
    return getChild()->bounds().map(getRasterMatrix());
  }

  Bounds viewportBoundsInRasterSpace() const
  {
    ASSERT(!isInvalid());
    ASSERT(viewport());
    return viewport()->bounds().map(getInversedLocalMatrix());
  }

protected:
  Bounds        onRevalidate(Revalidation* inv, const glm::mat3& ctm) override;
  Ref<Viewport> m_viewport;
  glm::mat3     m_rasterMatrix = glm::mat3{ 1.0 };
  glm::mat3     m_localMatrix = glm::mat3{ 1.0 };

  Viewport* viewport() const
  {
    return m_viewport.get();
  }

  GrRecordingContext* device() const
  {
    return m_device;
  }

  RasterManager::RasterExecutor* executor() const
  {
    return m_executor;
  }

  VGG_CLASS_MAKE(RasterNode);

private:
  GrRecordingContext*            m_device{ nullptr };
  RasterManager::RasterExecutor* m_executor{ nullptr };
};

std::vector<Bounds> mergeBounds(std::vector<Bounds> bounds);

} // namespace VGG::layer
