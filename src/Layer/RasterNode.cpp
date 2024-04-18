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

#include "Layer/Core/ZoomerNode.hpp"
#include "Renderer.hpp"

#include "Layer/Core/RasterNode.hpp"
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
  Ref<ClipEffectNode> viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     child)
  : TransformEffectNode(cnt, std::move(zoomer), std::move(child))
  , m_viewport(viewport)
  , m_raster(std::make_unique<RasterCacheTile>())
{
  observe(viewport); // TODO::
}

void RasterNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  auto rasterDevice = canvas->recordingContext();

  SkRect skv = toSkRect(getChild()->bounds());
  if (auto vp = m_viewport.lock())
  {
    skv = toSkRect(vp->bounds());
  }

  const auto z = asZoom(getTransform());
  int        lod = -1;

  std::visit(
    layer::Overloaded{ [&](ZoomerNode::EScaleLevel level) { lod = level; },
                       [&](ZoomerNode::OtherLevel other) { lod = -1; } },
    z->scaleLevel());

  auto                             mat = canvas->getTotalMatrix(); // DPI * zoom
  const auto                       skr = toSkRect(getChild()->bounds());
  const auto                       skm = toSkMatrix(z->getMatrix());
  layer::Rasterizer::RasterContext rasterCtx{ mat, m_picture.get(), &skr, skm };
  SkMatrix                         rasterMatrix;
  m_raster->rasterize(rasterDevice, rasterCtx, lod, skv, &m_rasterTiles, &rasterMatrix, 0);

  canvas->save();
  canvas->resetMatrix();
  canvas->setMatrix(rasterMatrix);
  for (auto& tile : m_rasterTiles)
  {
    canvas->drawImage(tile.image, tile.rect.left(), tile.rect.top());
    // SkPaint p;
    // p.setColor(SK_ColorRED);
    // p.setStyle(SkPaint::kStroke_Style);
    // canvas->drawRect(tile.rect, p);
  }
  canvas->restore();
}

Bounds RasterNode::onRevalidate()
{
  auto z = asZoom(getTransform());
  if (m_raster && z)
  {
    if (z->hasInvalScale())
    {
      m_raster->invalidate(layer::Rasterizer::EReason::ZOOM_SCALE);
    }

    if (z->hasOffsetInval())
    {
      m_raster->invalidate(layer::Rasterizer::EReason::ZOOM_TRANSLATION);
    }
  }
  return TransformEffectNode::revalidate();
}
} // namespace VGG::layer
