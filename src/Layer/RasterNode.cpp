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

#include "RasterManager.hpp"
#include "Renderer.hpp"
#include "DevCanvas.hpp"

#include "Layer/Formatters.hpp"
#include "Layer/Core/RasterNode.hpp"
#include "Layer/Core/RasterCache.hpp"
#include "Layer/Core/ViewportNode.hpp"

#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/RasterCacheTile.hpp"
#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <core/SkOverdrawCanvas.h>
#include <gpu/GrBackendSurface.h>
#include <gpu/GrRecordingContext.h>
#include <gpu/GrTypes.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <gpu/ganesh/SkImageGanesh.h>
#include <utils/SkNWayCanvas.h>

namespace
{
using namespace VGG::layer;
Ref<TransformNode> ensureTransformNode(Ref<TransformNode> viewport, Ref<TransformNode> zoomer)
{
  if (viewport && zoomer)
    return ConcateTransformNode::Make(std::move(viewport), std::move(zoomer));
  else if (viewport)
    return viewport;
  else if (zoomer)
    return zoomer;
  else
    return Matrix::Make();
}

} // namespace

namespace VGG::layer
{

RasterNode::RasterNode(
  VRefCnt*            cnt,
  GrRecordingContext* device,
  RasterExecutor*     executor,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     child)
  : TransformEffectNode(cnt, ensureTransformNode(viewport, std::move(zoomer)), std::move(child))
  , m_viewport(viewport)
  , m_device(device)
  , m_executor(executor)
{
}

Bounds RasterNode::onRevalidate(Revalidation* inv, const glm::mat3& ctm)
{
  auto bounds = TransformEffectNode::onRevalidate(inv, ctm);
  if (m_viewport)
    m_viewport->revalidate();

  auto t = getTransform();
  if (t)
  {
    const auto& totalMatrix = t->getMatrix();
    m_localMatrix[2][0] = totalMatrix[2][0];
    m_localMatrix[2][1] = totalMatrix[2][1];
    m_rasterMatrix = totalMatrix;
    m_rasterMatrix[2][0] = 0.0f;
    m_rasterMatrix[2][1] = 0.0f;
  }
  return bounds;
}

} // namespace VGG::layer
