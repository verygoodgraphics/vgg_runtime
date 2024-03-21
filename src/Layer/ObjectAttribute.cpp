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
#include "ObjectAttribute.hpp"

#include "PaintNodePrivate.hpp"

#include <core/SkImageFilter.h>
#include <effects/SkImageFilters.h>
#include "Effects.hpp"

namespace VGG::layer
{

Bound BackgroundBlurAttribute::onRevalidate()
{
  if (!m_blurs.empty())
  {
    for (const auto& b : m_blurs)
    {
      if (!b.isEnabled)
        continue;
      sk_sp<SkImageFilter> filter;
      filter = makeBackgroundBlurFilter(b.blur);
      if (m_imageFilter == nullptr)
      {
        m_imageFilter = filter;
      }
      else
      {
        m_imageFilter = SkImageFilters::Compose(m_imageFilter, filter);
      }
    }
  }
  return Bound();
}
void DropShadowAttribute::render(Renderer* renderer)
{
  if (m_dropShadowEffects && m_shapeAttr)
  {
    if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
    {
      m_dropShadowEffects->render(renderer, shape);
    }
  }
}

Bound DropShadowAttribute::onRevalidate()
{
  if (m_shapeAttr)
  {
    m_shapeAttr->revalidate();
    auto shape = m_shapeAttr->getShape();
    auto bounds = toSkRect(m_shapeAttr->bound());
    if (!m_shadow.empty() && !shape.isEmpty())
    {
      m_dropShadowEffects = DropShadowEffect(m_shadow, bounds, shape.outset(0, 0).has_value());
      SkRect shadowBounds = m_dropShadowEffects->bounds();
      return Bound{ shadowBounds.x(),
                    shadowBounds.y(),
                    shadowBounds.width(),
                    shadowBounds.height() };
    }
  }
  return Bound();
}

Bound InnerShadowAttribute::onRevalidate()
{
  if (m_shapeAttr)
  {
    m_shapeAttr->revalidate();
    auto shape = m_shapeAttr->getShape();
    if (shape.isEmpty())
      return Bound();
    auto shapeBounds = m_shapeAttr->getShape().bounds();
    if (!m_shadow.empty() && !shape.isEmpty())
    {
      m_innerShadowEffects = InnerShadowEffect(m_shadow, shapeBounds);
      // SkRect shadowBounds = m_innerShadowEffects->bounds();
      SkRect shadowBounds = shapeBounds;
      return Bound{ shadowBounds.x(),
                    shadowBounds.y(),
                    shadowBounds.width(),
                    shadowBounds.height() };
    }
  }
  return Bound();
}

void InnerShadowAttribute::render(Renderer* renderer)
{
  if (m_innerShadowEffects && m_shapeAttr)
  {
    if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
    {
      m_innerShadowEffects->render(renderer, shape);
    }
  }
}

SkRect ObjectAttribute::revalidateObjectBounds(
  const std::vector<Border>& borders,
  const SkRect&              bounds)
{
  const Border* maxWidthBorder = nullptr;
  float         maxWidth = 0;
  if (!bounds.isEmpty())
  {
    if (!m_borders.empty())
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
    }
    if (maxWidthBorder)
    {
      // Only consider these properties that affect bounds
      SkPaint paint;
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
      return rect;
    }
  }
  return bounds;
}

void ObjectAttribute::render(Renderer* renderer)
{
  ASSERT(renderer);
  sk_sp<SkBlender>     blender; // maybe we need to add a blender, add to renderer
  sk_sp<SkImageFilter> imageFilter;
  if (m_shapeAttr)
  {
    if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
    {
      ObjectRecorder rec;
      SkRect         objectBounds = shape.bounds();
      auto           recorder = rec.beginRecording(objectBounds, SkMatrix::I());
      auto           fillBounds = shape.bounds();
      FillEffect     fillEffect(m_fills, fillBounds, imageFilter, blender);
      fillEffect.render(renderer, shape);
      const auto borderBounds =
        internal_draw::drawBorder(recorder, shape, shape.bounds(), m_borders, blender);
      objectBounds.join(fillBounds);
      objectBounds.join(borderBounds);
      auto mat = SkMatrix::Translate(objectBounds.x(), objectBounds.y());
      m_styleDisplayList = rec.finishRecording(objectBounds, &mat);
    }
  }
}

Bound ObjectAttribute::onRevalidate()
{
  m_shapeAttr->revalidate();
  for (const auto& f : m_fills)
  {
    if (f.isEnabled)
    {
      m_hasFill = true;
      break;
    }
  }
  if (m_shapeAttr && !m_borders.empty())
  {
    const auto& shape = m_shapeAttr->getShape();
    auto        bounds = revalidateObjectBounds(m_borders, shape.bounds());
    return Bound{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
  }
  return Bound();
}

void StyleObjectAttribute::render(Renderer* renderer)
{
  auto filled = m_objectAttr->hasFill();
  if (filled && m_dropShadowAttr)
    m_dropShadowAttr->render(renderer);
  m_objectAttr->render(renderer);
  if (filled && m_innerShadowAttr)
    m_innerShadowAttr->render(renderer);
}

Bound StyleObjectAttribute::onRevalidate()
{
  m_innerShadowAttr->revalidate();
  m_backgroundBlurAttr->revalidate();
  auto objectBounds = toSkRect(m_objectAttr->revalidate());
  auto shadowBounds = toSkRect(m_dropShadowAttr->revalidate());
  objectBounds.join(shadowBounds);
  return Bound{ objectBounds.x(), objectBounds.y(), objectBounds.width(), objectBounds.height() };
}

} // namespace VGG::layer
