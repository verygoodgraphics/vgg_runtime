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

#include "Settings.hpp"
#include "DefaultRenderNode.hpp"
#include "AttributeAccessor.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "Effects.hpp"

#include "Layer/Memory/VAllocator.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

void DefaultRenderNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  canvas->drawPicture(m_picture);

  if (getDebugBoundEnable())
  {
    SkRect  renderBound = toSkRect(m_objectAttr->bound());
    SkRect  layerBound = toSkRect(m_alphaMaskAttr->bound());
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorBLUE);
    p.setStrokeWidth(3);
    renderer->canvas()->drawRect(renderBound, p);

    p.setColor(SK_ColorGREEN);
    p.setStrokeWidth(2);
    renderer->canvas()->drawRect(layerBound, p);
  }
  // recorder(renderer);
}

SkRect DefaultRenderNode::recorder(Renderer* renderer)
{
  sk_sp<SkImageFilter> dropbackFilter = m_objectAttr->getBackgroundBlurImageFilter();
  sk_sp<SkImageFilter> layerFXFilter = m_alphaMaskAttr->getImageFilter();
  const auto           newLayer = dropbackFilter || layerFXFilter;

  auto shapeMask = m_shapeMaskAttr->getShape();

  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->save();
    shapeMask.clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  SkRect renderBound = toSkRect(m_objectAttr->bound());
  SkRect layerBound = toSkRect(m_alphaMaskAttr->bound());
  if (newLayer)
  {
    SkPaint layerPaint;
    layerPaint.setAntiAlias(true);
    layerPaint.setImageFilter(layerFXFilter);
    VShape clipShape(layerBound);
    beginLayer(renderer, &layerPaint, &clipShape, dropbackFilter);
    renderBound = layerBound;
  }
  m_objectAttr->render(renderer);
  if (newLayer)
  {
    endLayer(renderer);
  }
  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->restore();
  }
  return renderBound;
}

void DefaultRenderNode::renderAsMask(Renderer* render)
{
  ASSERT(m_objectAttr);
  m_objectAttr->render(render);
}

std::pair<sk_sp<SkPicture>, SkRect> DefaultRenderNode::revalidatePicture(const SkRect& rect)
{
  ObjectRecorder rec;
  auto           renderer = rec.beginRecording(rect, SkMatrix::I());
  auto           r = recorder(renderer);
  return { rec.finishAsPicture(r), r };
}

void DefaultRenderNode::beginLayer(
  Renderer*            renderer,
  const SkPaint*       paint,
  const VShape*        clipShape,
  sk_sp<SkImageFilter> backdropFilter)
{
  renderer->canvas()->save();
  if (clipShape)
  {
    clipShape->clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  auto layerBound = clipShape->bounds();
  renderer->canvas()->saveLayer(
    SkCanvas::SaveLayerRec(&layerBound, paint, backdropFilter.get(), 0));
}

void DefaultRenderNode::endLayer(Renderer* renderer)
{
  renderer->canvas()->restore();
  renderer->canvas()->restore();
}

Bound DefaultRenderNode::onRevalidate()
{
  INFO("DefaultRenderNode::onRevalidate");
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  m_alphaMaskAttr->revalidate();
  m_objectAttr->revalidate();
  auto rect = toSkRect(m_objectAttr->bound());
  auto [pic, bound] = revalidatePicture(rect);
  m_picture = pic;
  return Bound{ bound.x(), bound.y(), bound.width(), bound.height() };
}

DefaultRenderNode::~DefaultRenderNode()
{
  unobserve(m_transformAttr);
  unobserve(m_objectAttr);
  unobserve(m_alphaMaskAttr);
  unobserve(m_shapeMaskAttr);
  // unobserve(m_shapeAttr);
}

} // namespace VGG::layer
