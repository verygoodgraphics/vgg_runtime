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

#include "RenderNode.hpp"
#include "AttributeAccessor.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "Effects.hpp"

#include "Layer/Memory/VAllocator.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

Ref<RenderNode> RenderNode::MakeFrom(VAllocator* alloc, PaintNode* node)
{
  auto shape = ShapeAttribute::Make(alloc);
  auto transform = TransformAttribute::Make(alloc);
  auto innerShadow = InnerShadowAttribute::Make(alloc, shape);
  auto dropShadow = DropShadowAttribute::Make(alloc, shape);
  auto backgroundBlur = BackgroundBlurAttribute::Make(alloc);
  auto object = ObjectAttribute::Make(alloc, shape);
  auto shapeMask = ShapeMaskAttribute::Make(alloc);

  auto style = StyleObjectAttribute::Make(alloc, innerShadow, dropShadow, object, backgroundBlur);
  auto layerPostProcess = LayerFXAttribute::Make(alloc, style);
  auto alphaMaskAttribute = AlphaMaskAttribute::Make(alloc, node, layerPostProcess);
  auto result = RenderNode::Make(
    alloc,
    transform,
    style,
    layerPostProcess,
    alphaMaskAttribute,
    shapeMask,
    shape);
  result->m_accessor = std::unique_ptr<AttributeAccessor>(new AttributeAccessor(
    transform,
    shape,
    alphaMaskAttribute,
    shapeMask,
    dropShadow,
    innerShadow,
    object,
    layerPostProcess,
    backgroundBlur));
  return result;
}

void RenderNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  auto skMatrix = toSkMatrix(m_transformAttr->getTransform().matrix());
  {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(skMatrix);
    canvas->drawPicture(m_picture);
  }
}

SkRect RenderNode::recorder(Renderer* renderer)
{
  auto       dropbackFilter = m_objectAttr->getBackgroundBlurImageFilter();
  auto       layerFXFilter = m_alphaMaskAttr->getImageFilter();
  const auto newLayer = dropbackFilter || layerFXFilter;

  auto path = m_shapeAttr->getShape();
  auto shapeMask = m_shapeMaskAttr->getShape();

  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->save();
    shapeMask.clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  SkRect renderBound = toSkRect(m_objectAttr->bound());
  if (newLayer)
  {
    SkRect  layerBound = toSkRect(m_alphaMaskAttr->bound());
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

std::pair<sk_sp<SkPicture>, SkRect> RenderNode::revalidatePicture(const SkRect& rect)
{
  ObjectRecorder rec;
  auto           renderer = rec.beginRecording(rect, SkMatrix::I());
  auto           r = recorder(renderer);
  return { rec.finishAsPicture(r), r };
}

void RenderNode::beginLayer(
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

void RenderNode::endLayer(Renderer* renderer)
{
  renderer->canvas()->restore();
  renderer->canvas()->restore();
}

Bound RenderNode::onRevalidate()
{
  m_shapeAttr->revalidate();
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  m_alphaMaskAttr->revalidate();
  m_objectAttr->revalidate();
  auto rect = toSkRect(m_objectAttr->bound());
  auto [pic, bound] = revalidatePicture(rect);
  return Bound{ bound.x(), bound.y(), bound.width(), bound.height() };
}

RenderNode::~RenderNode()
{
  unobserve(m_transformAttr);
  unobserve(m_objectAttr);
  unobserve(m_alphaMaskAttr);
  unobserve(m_shapeMaskAttr);
  unobserve(m_shapeAttr);
}

} // namespace VGG::layer
