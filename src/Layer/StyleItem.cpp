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
#include "Layer/Config.hpp"
#include "Layer/Core/EffectNode.hpp"
#include "Layer/ShapeItem.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "Effects.hpp"

#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Memory/VAllocator.hpp"
#include <core/SkCanvas.h>

namespace
{

} // namespace

namespace VGG::layer
{

StyleItem::StyleItem(
  VRefCnt*                cnt,
  Ref<TransformAttribute> transform,
  Ref<StyleAttribute>     styleObject,
  Ref<LayerFXAttribute>   layerPostProcess,
  Ref<AlphaMaskAttribute> alphaMask,
  Ref<ShapeMaskAttribute> shapeMask,
  Ref<ShapeAttribute>     shape)
  : GraphicItem(cnt)
  , m_transformAttr(transform)
  , m_styleAttr(styleObject)
  , m_alphaMaskAttr(alphaMask)
  , m_shapeMaskAttr(shapeMask)
{
  observe(m_transformAttr);
  observe(m_styleAttr);
  observe(m_alphaMaskAttr);
  observe(m_shapeMaskAttr);
}

void StyleItem::render(Renderer* renderer)
{
  VGG_LAYER_DEBUG_CODE(if (!m_picture) VGG_LOG_DEV(ERROR, StyleItem, "no picture"););
  // auto canvas = renderer->canvas();
  // canvas->drawPicture(m_picture);
  recorder(renderer);
}

#ifdef VGG_LAYER_DEBUG
void StyleItem::debug(Renderer* render)
{
  auto   canvas = render->canvas();
  SkRect layerBounds = toSkRect(m_alphaMaskAttr->bounds());
  SkRect effectBounds = toSkRect(m_styleAttr->effectBounds());

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

void StyleItem::recorder(Renderer* renderer)
{
  auto       shapeMask = m_shapeMaskAttr->getShape();
  const auto newLayer = hasNewLayer();

  SkAutoCanvasRestore acr(renderer->canvas(), !shapeMask.isEmpty());
  if (!shapeMask.isEmpty())
  {
    shapeMask.clip(renderer->canvas(), SkClipOp::kIntersect);
  }
  AutoLayerRestore alr(
    renderer,
    newLayer,
    [this](SkPaint& p, VShape& shape, sk_sp<SkImageFilter>& backdropFilter)
    {
      p.setAntiAlias(true);
      p.setImageFilter(m_alphaMaskAttr->getImageFilter());
      shape = VShape(toSkRect(effectBounds()));
      backdropFilter = m_styleAttr->getBackdropImageFilter();
    });
  m_styleAttr->render(renderer);
}

void StyleItem::renderAsMask(Renderer* render)
{
  ASSERT(m_styleAttr);
  m_styleAttr->render(render);
}

bool StyleItem::hasNewLayer() const
{
  sk_sp<SkImageFilter> dropbackFilter = m_styleAttr->getBackdropImageFilter();
  sk_sp<SkImageFilter> layerFXFilter = m_alphaMaskAttr->getImageFilter();
  const auto           newLayer = dropbackFilter || layerFXFilter;
  return newLayer;
}

void StyleItem::revalidateEffectsBounds()
{
  m_effectsBounds = m_styleAttr->effectBounds();
  if (hasNewLayer())
  {
    m_effectsBounds = m_alphaMaskAttr->bounds();
  }
}

Bounds StyleItem::effectBounds() const
{
  return m_effectsBounds;
}

void StyleItem::setFillStyle(const std::vector<Fill>& fills)
{
  if (m_fills == fills)
    return;
  m_fills = fills;
  if (m_fills.empty())
  {
    m_fillEffect = nullptr;
  }
  else
  {
  }
}

void StyleItem::setFillStyle(std::remove_cvref_t<const std::vector<Fill>&>&& fills)
{
  if (m_fills == fills)
    return;
  m_fills = fills;
}

const std::vector<Fill>& StyleItem::getFillStyle() const
{
  return m_fills;
}

void StyleItem::setBorderStyle(const std::vector<Border>& borders)
{
  if (m_borders == borders)
    return;
  m_borders = borders;
}

void StyleItem::setBorderStyle(std::remove_cvref_t<const std::vector<Border>&>&& borders)
{
  m_borders = borders;
}

const std::vector<Border>& StyleItem::getBorderStyle() const
{
  return m_borders;
}

std::pair<sk_sp<SkPicture>, SkRect> StyleItem::revalidatePicture(const SkRect& rect)
{
  ObjectRecorder rec;
  auto           renderer = rec.beginRecording(rect, SkMatrix::I());
  recorder(renderer);
  return { rec.finishAsPicture(rect), rect };
}

Bounds StyleItem::onRevalidate()
{
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  m_alphaMaskAttr->revalidate();
  m_styleAttr->revalidate();
  revalidateEffectsBounds();
  // auto [pic, bounds] = revalidatePicture(toSkRect(m_objectAttr->effectBounds()));
  // m_effectsBounds = Bounds{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
  // m_picture = std::move(pic);
  return m_styleAttr->bounds();
}

StyleItem::~StyleItem()
{
  unobserve(m_transformAttr);
  unobserve(m_styleAttr);
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
