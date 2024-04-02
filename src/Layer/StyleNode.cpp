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
#include "StyleNode.hpp"
#include "AttributeAccessor.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "Effects.hpp"

#include "Layer/Memory/VAllocator.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

void StyleNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  canvas->drawPicture(m_picture);

  if (getDebugBoundEnable())
  {
    SkRect layerBound = toSkRect(m_alphaMaskAttr->bound());
    SkRect effectBounds = toSkRect(m_objectAttr->effectBounds());

    SkPaint p;
    p.setStroke(true);
    p.setColor(SK_ColorGREEN);
    p.setStrokeWidth(2);
    renderer->canvas()->drawRect(layerBound, p);

    p.setColor(SK_ColorBLUE);
    const SkScalar intervals[2] = { 10, 20 };
    p.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
    p.setStrokeWidth(2);
    renderer->canvas()->drawRect(effectBounds, p);
  }
  // recorder(renderer);
}

SkRect StyleNode::recorder(Renderer* renderer)
{
  sk_sp<SkImageFilter> dropbackFilter = m_objectAttr->getBackdropImageFilter();
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

void StyleNode::renderAsMask(Renderer* render)
{
  ASSERT(m_objectAttr);
  m_objectAttr->render(render);
}

Bounds StyleNode::effectBounds() const
{
  return m_effectsBounds;
}

std::pair<sk_sp<SkPicture>, SkRect> StyleNode::revalidatePicture(const SkRect& rect)
{
  ObjectRecorder rec;
  auto           renderer = rec.beginRecording(rect, SkMatrix::I());
  auto           r = recorder(renderer);
  return { rec.finishAsPicture(r), r };
}

void StyleNode::beginLayer(
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

void StyleNode::endLayer(Renderer* renderer)
{
  renderer->canvas()->restore();
  renderer->canvas()->restore();
}

Bounds StyleNode::onRevalidate()
{
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  m_alphaMaskAttr->revalidate();
  m_objectAttr->revalidate();
  auto rect = toSkRect(m_objectAttr->bound());
  auto [pic, bound] = revalidatePicture(rect);
  m_effectsBounds = Bounds{ bound.x(), bound.y(), bound.width(), bound.height() };
  m_picture = pic;
  return m_objectAttr->bound();
}

StyleNode::~StyleNode()
{
  unobserve(m_transformAttr);
  unobserve(m_objectAttr);
  unobserve(m_alphaMaskAttr);
  unobserve(m_shapeMaskAttr);
  // unobserve(m_shapeAttr);
}

std::pair<Ref<StyleNode>, std::unique_ptr<Accessor>> StyleNode::MakeRenderNode( // NOLINT
  VAllocator*             alloc,
  PaintNode*              node,
  Ref<TransformAttribute> transform,
  Creator                 creator)
{
  auto backgroundBlur = BackdropFXAttribute::Make(alloc);
  auto object = ObjectAttribute::Make(alloc, Ref<InnerObjectAttribute>());
  auto renderObject = creator(alloc, object.get());
  auto shape = incRef(renderObject->shape());
  auto innerShadow = InnerShadowAttribute::Make(alloc, shape);
  auto dropShadow = DropShadowAttribute::Make(alloc, shape);
  object->setRenderObject(renderObject);

  auto style = StyleAttribute::Make(alloc, innerShadow, dropShadow, object, backgroundBlur);
  auto layerPostProcess = LayerFXAttribute::Make(alloc, style);
  auto shapeMask = ShapeMaskAttribute::Make(alloc, node, layerPostProcess);
  auto alphaMaskAttribute = AlphaMaskAttribute::Make(alloc, node, layerPostProcess);
  auto result = StyleNode::Make(
    alloc,
    transform,
    style,
    layerPostProcess,
    alphaMaskAttribute,
    shapeMask,
    shape);
  auto aa = std::unique_ptr<Accessor>(new Accessor(
    transform,
    alphaMaskAttribute,
    shapeMask,
    dropShadow,
    innerShadow,
    object,
    layerPostProcess,
    backgroundBlur));
  return { result, std::move(aa) };
}

} // namespace VGG::layer
