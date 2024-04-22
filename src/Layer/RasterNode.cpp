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

#include "Layer/Core/RasterCache.hpp"
#include "Renderer.hpp"
#include "Layer/RasterNode.hpp"
#include "Layer/ViewportNode.hpp"

#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/RasterCacheTile.hpp"

namespace
{

using namespace VGG::layer;

ZoomerNode* asZoom(TransformNode* node)
{
  return static_cast<ZoomerNode*>(node);
}

} // namespace

namespace VGG::layer
{

RasterNode::RasterNode(
  VRefCnt*            cnt,
  GrRecordingContext* device,
  Ref<ViewportNode>   viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     child)
  : TransformEffectNode(cnt, std::move(zoomer), std::move(child))
  , m_viewport(viewport)
  , m_raster(std::make_unique<RasterCacheTile>())
{
  m_device = device;
  ASSERT(getChild());
  ASSERT(m_device);
  ASSERT(m_raster);
  observe(viewport);
}

void RasterNode::render(Renderer* renderer)
{
  auto c = getChild();
  ASSERT(c);
  if (!c->picture())
  {
    c->render(renderer);
  }
  else
  {
    auto canvas = renderer->canvas();
    ASSERT(canvas);
    canvas->save();
    canvas->resetMatrix();
    canvas->setMatrix(m_rasterMatrix);
    for (auto& tile : m_rasterTiles)
    {
      canvas->drawImage(tile.image, tile.rect.left(), tile.rect.top());
      SkPaint p;
      p.setColor(SK_ColorRED);
      p.setStyle(SkPaint::kStroke_Style);
      canvas->drawRect(tile.rect, p);
    }
    canvas->restore();
  }
}

Bounds RasterNode::onRevalidate()
{
  auto c = getChild();
  ASSERT(c);
  auto   z = asZoom(getTransform());
  bool   needRaster = false;
  Bounds finalBounds;

  // FIXME: you cannot determine the invalidation by hasInval**() functions,
  // because it could be revaildated by other nodes. unless it is exclusive to this node.
  if (m_raster)
  {
    if (c->hasInval())
    {
      m_raster->invalidate(layer::Rasterizer::EReason::CONTENT);
      needRaster = true;
      c->revalidate();
    }
    if (z)
    {
      if (z->hasInvalScale())
      {
        m_raster->invalidate(layer::Rasterizer::EReason::ZOOM_SCALE);
        needRaster = true;
      }
      if (z->hasOffsetInval())
      {
        m_raster->invalidate(layer::Rasterizer::EReason::ZOOM_TRANSLATION);
        needRaster = true;
      }
      z->revalidate();
    }
    if (m_viewport && m_viewport->hasInvalidate())
    {
      m_raster->invalidate(layer::Rasterizer::EReason::VIEWPORT);
      m_viewport->revalidate();
      needRaster = true;
      finalBounds = m_viewport->bounds();
    }
  }

  if (needRaster)
  {
    ASSERT(m_device);
    SkRect skv = toSkRect(getChild()->bounds());
    if (m_viewport)
    {
      skv = toSkRect(m_viewport->getViewport());
    }

    int lod = -1;
    if (z)
    {
      std::visit(
        layer::Overloaded{ [&](ZoomerNode::EScaleLevel level) { lod = level; },
                           [&](ZoomerNode::OtherLevel other) { lod = -1; } },
        z->scaleLevel());
    }

    if (c->picture())
    {
      const auto                localMatrix = SkMatrix::I();
      const auto                deviceMatrix = toSkMatrix(m_viewport->getMatrix() * z->getMatrix());
      const auto                contentBounds = toSkRect(c->bounds());
      Rasterizer::RasterContext rasterCtx{ deviceMatrix,
                                           c->picture(),
                                           &contentBounds,
                                           localMatrix };
      m_raster->rasterize(m_device, rasterCtx, lod, skv, &m_rasterTiles, &m_rasterMatrix, 0);
    }
  }
  return finalBounds;
}
} // namespace VGG::layer
