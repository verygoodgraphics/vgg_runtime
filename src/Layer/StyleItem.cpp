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

#include "StyleItem.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "Effects.hpp"

#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include <core/SkCanvas.h>

namespace VGG::layer
{

void StyleItem::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  canvas->drawPicture(m_picture);
  // recorder(renderer);
}

#ifdef VGG_LAYER_DEBUG
void StyleItem::debug(Renderer* render)
{
  auto   canvas = render->canvas();
  SkRect layerBounds = toSkRect(m_alphaMaskAttr->bounds());
  SkRect effectBounds = toSkRect(m_objectAttr->effectBounds());

  SkPaint p;
  p.setStroke(true);
  p.setColor(SK_ColorGREEN);
  p.setStrokeWidth(2);
  canvas->drawRect(layerBounds, p);

  p.setColor(SK_ColorBLUE);
  const SkScalar intervals[2] = { 10, 20 };
  p.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
  p.setStrokeWidth(2);
  canvas->drawRect(effectBounds, p);
}
#endif

SkRect StyleItem::recorder(Renderer* renderer)
{
  auto shapeMask = m_shapeMaskAttr->getShape();
  if (!shapeMask.isEmpty())
  {
    renderer->canvas()->save();
    shapeMask.clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  const auto newLayer = hasNewLayer();
  SkRect     effectsBounds = toSkRect(m_objectAttr->effectBounds());
  if (newLayer)
  {
    SkPaint layerPaint;
    layerPaint.setAntiAlias(true);
    layerPaint.setImageFilter(m_alphaMaskAttr->getImageFilter());
    effectsBounds = toSkRect(m_alphaMaskAttr->bounds());
    VShape clipShape(effectsBounds);
    beginLayer(renderer, &layerPaint, &clipShape, m_objectAttr->getBackdropImageFilter());
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
  return effectsBounds;
}

void StyleItem::renderAsMask(Renderer* render)
{
  ASSERT(m_objectAttr);
  m_objectAttr->render(render);
}

bool StyleItem::hasNewLayer() const
{
  sk_sp<SkImageFilter> dropbackFilter = m_objectAttr->getBackdropImageFilter();
  sk_sp<SkImageFilter> layerFXFilter = m_alphaMaskAttr->getImageFilter();
  const auto           newLayer = dropbackFilter || layerFXFilter;
  return newLayer;
}

Bounds StyleItem::effectBounds() const
{
  return m_effectsBounds;
}

std::pair<sk_sp<SkPicture>, SkRect> StyleItem::revalidatePicture(const SkRect& rect)
{
  ObjectRecorder rec;
  auto           renderer = rec.beginRecording(rect, SkMatrix::I());
  auto           r = recorder(renderer);
  return { rec.finishAsPicture(r), r };
}

void StyleItem::beginLayer(
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
  auto layerBounds = clipShape->bounds();
  renderer->canvas()->saveLayer(
    SkCanvas::SaveLayerRec(&layerBounds, paint, backdropFilter.get(), 0));
}

void StyleItem::endLayer(Renderer* renderer)
{
  renderer->canvas()->restore();
  renderer->canvas()->restore();
}

Bounds StyleItem::onRevalidate()
{
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  m_alphaMaskAttr->revalidate();
  m_objectAttr->revalidate();
  auto [pic, bounds] = revalidatePicture(toSkRect(m_objectAttr->effectBounds()));
  m_effectsBounds = Bounds{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
  m_picture = std::move(pic);
  return m_objectAttr->bounds();
}

StyleItem::~StyleItem()
{
  unobserve(m_transformAttr);
  unobserve(m_objectAttr);
  unobserve(m_alphaMaskAttr);
  unobserve(m_shapeMaskAttr);
  // unobserve(m_shapeAttr);
}

std::pair<Ref<StyleItem>, std::unique_ptr<Accessor>> StyleItem::MakeRenderNode( // NOLINT
  VAllocator*             alloc,
  PaintNode*              node,
  Ref<TransformAttribute> transform,
  Creator                 creator)
{
  auto backgroundBlur = BackdropFXAttribute::Make(alloc);
  auto object = ObjectAttribute::Make(alloc, Ref<GraphicItem>());
  auto renderObject = creator(alloc, object.get());
  auto shape = incRef(renderObject->shape());
  auto innerShadow = InnerShadowAttribute::Make(alloc, shape);
  auto dropShadow = DropShadowAttribute::Make(alloc, shape);
  object->setGraphicItem(renderObject);

  auto style = StyleAttribute::Make(alloc, innerShadow, dropShadow, object, backgroundBlur);
  auto layerPostProcess = LayerFXAttribute::Make(alloc, style);
  auto shapeMask = ShapeMaskAttribute::Make(alloc, node, layerPostProcess);
  auto alphaMaskAttribute = AlphaMaskAttribute::Make(alloc, node, layerPostProcess);
  auto result = StyleItem::Make(
    alloc,
    transform,
    style,
    layerPostProcess,
    alphaMaskAttribute,
    shapeMask,
    shape);
  auto aa = std::unique_ptr<Accessor>(new Accessor(
    node,
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
