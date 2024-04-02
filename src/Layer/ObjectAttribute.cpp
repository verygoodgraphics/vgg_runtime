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

#include "VSkia.hpp"
#include "Effects.hpp"
#include "ObjectAttribute.hpp"
#include "ShapeAttribute.hpp"
#include "PaintNodePrivate.hpp"
#include "EffectAttribute.hpp"

#include <core/SkImageFilter.h>
#include <effects/SkImageFilters.h>

namespace VGG::layer
{

std::pair<SkRect, std::optional<SkPaint>> ObjectAttribute::revalidateObjectBounds(
  const std::vector<Border>& borders,
  const SkRect&              bounds)
{
  const Border* maxWidthBorder = nullptr;
  float         maxWidth = 0;
  if (!bounds.isEmpty())
  {
    for (const auto& b : borders)
    {
      if (!b.isEnabled || b.thickness <= 0)
        continue;
      // We simply assumes that the wider of the stroke, the larger its bounds
      float strokeWidth = b.thickness;
      if (b.position == PP_INSIDE)
        strokeWidth = 2.f * b.thickness;
      else if (b.position == PP_OUTSIDE)
        strokeWidth = 2.f * b.thickness;
      if (strokeWidth > maxWidth)
      {
        maxWidth = strokeWidth;
        maxWidthBorder = &b;
      }
    }
    if (maxWidthBorder)
    {
      SkPaint paint;
      // Only consider these properties that affect bounds
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setAntiAlias(true);
      paint.setPathEffect(SkDashPathEffect::Make(
        maxWidthBorder->dashedPattern.data(),
        maxWidthBorder->dashedPattern.size(),
        maxWidthBorder->dashedOffset));
      paint.setStrokeJoin(toSkPaintJoin(maxWidthBorder->lineJoinStyle));
      paint.setStrokeCap(toSkPaintCap(maxWidthBorder->lineCapStyle));
      paint.setStrokeMiter(maxWidthBorder->miterLimit);
      paint.setStrokeWidth(maxWidthBorder->thickness);
      paint.setStrokeWidth(maxWidth);
      SkRect rect;
      paint.computeFastStrokeBounds(bounds, &rect);
      return { rect, paint };
    }
  }
  return { bounds, std::nullopt };
}

void ObjectAttribute::revalidateMaskFilter(const SkPaint& strokePaint, const SkRect& bounds)
{
}

void ObjectAttribute::render(Renderer* renderer)
{
  ASSERT(renderer);
  sk_sp<SkBlender>     blender; // maybe we need to add a blender, add to renderer
  sk_sp<SkImageFilter> imageFilter;
  m_graphicItem->render(renderer);
  // if (m_shapeAttr)
  // {
  //   if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
  //   {
  //     ObjectRecorder rec;
  //     SkRect         objectBounds = shape.bounds();
  //     auto           recorder = rec.beginRecording(objectBounds, SkMatrix::I());
  //     auto           fillBounds = shape.bounds();
  //     FillEffect     fillEffect(m_fills, fillBounds, imageFilter, blender);
  //     fillEffect.render(renderer, shape);
  //     const auto borderBounds =
  //       internal::drawBorder(recorder, shape, shape.bounds(), m_borders, blender);
  //     objectBounds.join(fillBounds);
  //     objectBounds.join(borderBounds);
  //     auto mat = SkMatrix::Translate(objectBounds.x(), objectBounds.y());
  //     m_styleDisplayList = rec.finishRecording(objectBounds, &mat);
  //     // m_styleDisplayList->render(renderer);
  //     SkPaint p;
  //     p.setStyle(SkPaint::kFill_Style);
  //     p.setAntiAlias(true);
  //     p.setColor(SK_ColorRED);
  //     p.setAlphaf(0.5f);
  //     shape.draw(recorder->canvas(), p);
  //   }
  // }
}

Bounds ObjectAttribute::onRevalidate()
{
  ASSERT(m_graphicItem);
  m_graphicItem->revalidate();
  m_hasFill = false;
  for (const auto& f : m_fills)
  {
    if (f.isEnabled)
    {
      m_hasFill = true;
      break;
    }
  }
  // m_effectBounds =
  //   m_renderObjectAttr->effectBounds(); // FIXME: maybe this could be done in the current
  // object rather than m_renderObjectAttr
  return m_graphicItem->bound();
  // m_shapeAttr->revalidate();
  // if (m_shapeAttr)
  // {
  //   const auto& shape = m_shapeAttr->getShape();
  //   auto [bounds, paint] = revalidateObjectBounds(m_borders, shape.bounds());
  //   ObjectRecorder rec;
  //   auto           recorder = rec.beginRecording(bounds, SkMatrix::I());
  //   SkPaint        fillPaint;
  //   fillPaint.setAntiAlias(true);
  //   fillPaint.setStyle(SkPaint::kFill_Style);
  //   fillPaint.setAlphaf(1.0f);
  //   shape.draw(recorder->canvas(), fillPaint);
  //   if (auto strokePen = paint; strokePen)
  //   {
  //     strokePen->setAlphaf(1.0f);
  //     shape.draw(recorder->canvas(), *strokePen);
  //   }
  //   auto mat = SkMatrix::Translate(bounds.x(), bounds.y());
  //   auto object = rec.finishRecording(bounds, &mat);
  //   m_maskFilter = object.asImageFilter();
  //   ASSERT(m_maskFilter);
  //   return Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
  // }
  // return Bound();
}

StyleAttribute::StyleAttribute(
  VRefCnt*                  cnt,
  Ref<InnerShadowAttribute> innerShadow,
  Ref<DropShadowAttribute>  dropShadow,
  Ref<ObjectAttribute>      object,
  Ref<BackdropFXAttribute>  backgroundBlur)
  : Attribute(cnt)
  , m_innerShadowAttr(std::move(innerShadow))
  , m_dropShadowAttr(std::move(dropShadow))
  , m_objectAttr(std::move(object))
  , m_backgroundBlurAttr(std::move(backgroundBlur))
{
  observe(m_innerShadowAttr);
  observe(m_dropShadowAttr);
  observe(m_objectAttr);
  observe(m_backgroundBlurAttr);
}

void StyleAttribute::render(Renderer* renderer)
{
  auto filled = m_objectAttr->hasFill();
  if (filled && m_dropShadowAttr)
    m_dropShadowAttr->render(renderer);
  m_objectAttr->render(renderer);
  if (filled && m_innerShadowAttr)
    m_innerShadowAttr->render(renderer);
}

void StyleAttribute::revalidateDropbackFilter(const SkRect& bounds)
{
  ASSERT(m_backgroundBlurAttr);
  m_backgroundBlurAttr->revalidate();
  if (auto bgb = m_backgroundBlurAttr->getImageFilter())
  {
    if (m_bgBlurImageFilter != bgb || m_objectEffectBounds != bounds)
    {
      m_bgBlurImageFilter = bgb;
      if (m_bgBlurImageFilter)
      {
        if (auto ro = m_objectAttr->getGraphicItem(); ro)
        {
          static auto s_blender = getOrCreateBlender("maskOut", g_maskOutBlender);
          auto        fb = ro->getMaskFilter();
          m_dropbackImageFilter = SkImageFilters::Blend(s_blender, fb, m_bgBlurImageFilter, bounds);
        }
      }
    }
    else
    {
      m_dropbackImageFilter = nullptr;
    }
  }
}

Bounds StyleAttribute::onRevalidate()
{
  m_innerShadowAttr->revalidate();
  m_objectAttr->revalidate();
  auto objectBounds = toSkRect(m_objectAttr->effectBounds());
  revalidateDropbackFilter(objectBounds);
  auto shadowBounds = toSkRect(m_dropShadowAttr->revalidate());
  objectBounds.join(shadowBounds);
  m_effectBounds =
    Bounds{ objectBounds.x(), objectBounds.y(), objectBounds.width(), objectBounds.height() };
  return m_objectAttr->bound();
}

} // namespace VGG::layer