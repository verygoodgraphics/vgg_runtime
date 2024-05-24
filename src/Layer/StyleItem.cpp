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
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Config.hpp"
#include "Layer/Core/EffectNode.hpp"
#include "Layer/EffectAttribute.hpp"
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

class StyleItem__pImpl
{
  VGG_DECL_API(StyleItem);

public:
  StyleItem__pImpl(StyleItem* api)
    : q_ptr(api)
  {
  }

  sk_sp<SkImageFilter> itemMaskFilter;

  void revalidateMaskFilter()
  {
    const auto& s = q_ptr->m_graphicItem->shape();
    if (s)
    {
      if (const auto& shape = s->getShape(); !shape.isEmpty())
      {
        auto           bounds = toSkRect(q_ptr->m_borderEffect->effectBounds());
        ObjectRecorder rec;
        auto           recorder = rec.beginRecording(bounds, SkMatrix::I());
        SkPaint        fillPaint;
        fillPaint.setAntiAlias(true);
        fillPaint.setStyle(SkPaint::kFill_Style);
        fillPaint.setAlphaf(1.0f);
        shape.draw(recorder->canvas(), fillPaint);
        SkPaint strokePen;
        strokePen.setAntiAlias(true);
        strokePen.setStyle(SkPaint::kStroke_Style);
        strokePen.setAlphaf(1.0f);
        shape.draw(recorder->canvas(), strokePen);
        auto mat = SkMatrix::Translate(bounds.x(), bounds.y());
        auto object = rec.finishRecording(bounds, &mat);
        itemMaskFilter = object.asImageFilter();
      }
    }
    else
    {
      itemMaskFilter = nullptr;
    }
  }
};

StyleItem::StyleItem(
  VRefCnt*                cnt,
  PaintNode*              node,
  Ref<TransformAttribute> transform,
  Creator                 creator)
  : GraphicItem(cnt)
  , d_ptr(new StyleItem__pImpl(this))
  , m_transformAttr(transform)
{

  Ref<LayerFXAttribute> layerPostProcess = LayerFXAttribute::Make(WeakRef<StyleItem>(this));
  m_shapeMaskAttr = ShapeMaskAttribute::Make(node, layerPostProcess);
  m_alphaMaskAttr = AlphaMaskAttribute::Make(node, layerPostProcess);
  m_graphicItem = creator(nullptr, this);
  auto shape = incRef(m_graphicItem->shape());
  // m_objectAttr->setGraphicItem(renderObject);
  m_innerShadowAttr = InnerShadowAttribute::Make(shape);
  m_dropShadowAttr = DropShadowAttribute::Make(shape);
  m_backgroundBlurAttr = BackdropFXAttribute::Make();

  m_fillEffect = StackFillEffectImpl::Make(m_graphicItem);
  m_borderEffect = StackBorderEffectImpl::Make(m_graphicItem);

  observe(m_transformAttr);
  observe(m_alphaMaskAttr);
  observe(m_shapeMaskAttr);
  observe(m_fillEffect);
  observe(m_borderEffect);
  observeStyleAttribute();
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
  SkRect effectBounds = toSkRect(styleEffectBounds());

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
      backdropFilter = backdropImageFilter();
    });
  onRenderStyle(renderer);
}

void StyleItem::onRenderStyle(Renderer* renderer)
{
  if (m_hasFill && m_dropShadowAttr)
    m_dropShadowAttr->render(renderer);

  m_graphicItem->render(renderer);
  m_fillEffect->render(renderer);
  m_borderEffect->render(renderer);

  if (m_hasFill && m_innerShadowAttr)
    m_innerShadowAttr->render(renderer);
}

void StyleItem::renderAsMask(Renderer* render)
{
  onRenderStyle(render);
}

bool StyleItem::hasNewLayer() const
{
  sk_sp<SkImageFilter> dropbackFilter = backdropImageFilter();
  sk_sp<SkImageFilter> layerFXFilter = m_alphaMaskAttr->getImageFilter();
  const auto           newLayer = dropbackFilter || layerFXFilter;
  return newLayer;
}

void StyleItem::revalidateEffectsBounds()
{
  m_effectsBounds = styleEffectBounds();
  if (hasNewLayer())
  {
    m_effectsBounds = m_alphaMaskAttr->bounds();
  }
}

Bounds StyleItem::effectBounds() const
{
  return m_effectsBounds;
}

std::pair<sk_sp<SkPicture>, SkRect> StyleItem::revalidatePicture(const SkRect& rect)
{
  ObjectRecorder rec;
  auto           renderer = rec.beginRecording(rect, SkMatrix::I());
  recorder(renderer);
  return { rec.finishAsPicture(rect), rect };
}

Bounds StyleItem::onRevalidateObject()
{
  m_graphicItem->revalidate();
  m_hasFill = false;
  for (const auto& p : m_fillEffect->fills())
  {
    const auto& f = p->getFill();
    if (f.isEnabled)
    {
      m_hasFill = true;
      break;
    }
  }
  return m_graphicItem->bounds();
}

Bounds StyleItem::onRevalidateStyle()
{
  m_fillEffect->revalidate();
  m_borderEffect->revalidate();
  m_innerShadowAttr->revalidate();

  onRevalidateObject();

  auto objectBounds = toSkRect(m_borderEffect->effectBounds());
  revalidateDropbackFilter(objectBounds);
  auto shadowBounds = toSkRect(m_dropShadowAttr->revalidate());
  objectBounds.join(shadowBounds);
  m_styleEffectBounds =
    Bounds{ objectBounds.x(), objectBounds.y(), objectBounds.width(), objectBounds.height() };
  return m_graphicItem->bounds();
}

void StyleItem::observeStyleAttribute()
{
  observe(m_innerShadowAttr);
  observe(m_dropShadowAttr);
  observe(m_backgroundBlurAttr);
}

void StyleItem::unobserveStyleAttribute()
{
  unobserve(m_innerShadowAttr);
  unobserve(m_dropShadowAttr);
  unobserve(m_backgroundBlurAttr);
}

void StyleItem::revalidateDropbackFilter(const SkRect& bounds)
{
  ASSERT(m_backgroundBlurAttr);
  if (m_hasFill) // Dropback effect determined by the wheather the
                 // object has fill
  {
    m_backgroundBlurAttr->revalidate();
    if (auto bgb = m_backgroundBlurAttr->getImageFilter())
    {
      if (m_bgBlurImageFilter != bgb || m_objectEffectBounds != bounds)
      {
        m_bgBlurImageFilter = bgb;
        if (m_bgBlurImageFilter)
        {
          if (auto ro = m_graphicItem; ro)
          {
            static auto s_blender = getOrCreateBlender("maskOut", g_maskOutBlender);
            d_ptr->revalidateMaskFilter();
            auto fb = d_ptr->itemMaskFilter;
            m_dropbackImageFilter =
              SkImageFilters::Blend(s_blender, fb, m_bgBlurImageFilter, bounds);
          }
        }
      }
      else
      {
        m_dropbackImageFilter = nullptr;
      }
    }
  }
}

Bounds StyleItem::objectBounds()
{
  return m_graphicItem->bounds();
}

Bounds StyleItem::onRevalidate()
{
  auto bounds = onRevalidateStyle(); // this must be the first, workaround
  m_shapeMaskAttr->revalidate();
  m_transformAttr->revalidate();
  m_alphaMaskAttr->revalidate();
  // m_styleAttr->revalidate();
  revalidateEffectsBounds();
  // auto [pic, bounds] = revalidatePicture(toSkRect(m_objectAttr->effectBounds()));
  // m_effectsBounds = Bounds{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
  // m_picture = std::move(pic);
  return bounds;
}

StyleItem::~StyleItem()
{
  unobserve(m_transformAttr);
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
  auto result = StyleItem::Make(alloc, node, transform, creator);

  auto aa = std::unique_ptr<Accessor>(new Accessor(
    node,
    result.get(),
    transform,
    result->m_alphaMaskAttr,
    result->m_shapeMaskAttr,
    result->m_dropShadowAttr,
    result->m_innerShadowAttr,
    result->m_shapeMaskAttr->layerFX(),
    result->m_backgroundBlurAttr));
  return { result, std::move(aa) };
}

} // namespace VGG::layer
