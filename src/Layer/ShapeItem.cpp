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
#include "ShapeItem.hpp"
#include "Effects.hpp"
#include "Layer/StyleItem.hpp"

namespace VGG::layer
{

void ShapeItem::render(Renderer* renderer)
{
  return;
}

ShapeAttribute* ShapeItem::shape() const
{
  return m_shapeAttr.get();
}

std::pair<SkRect, std::optional<SkPaint>> ShapeItem::revalidateObjectBounds(
  const std::vector<Border>& borders,
  const SkRect&              bounds)
{
  const Border* maxWidthBorder = nullptr;
  float         maxWidth = 0;
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
  return { bounds, std::nullopt };
}

Bounds ShapeItem::onRevalidate(Revalidation* inv, const glm::mat3& mat)
{
  ASSERT(m_shapeAttr);
  m_shapeAttr->revalidate();
  if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
  {
    const auto rect = shape.bounds();
    return Bounds{ rect.x(), rect.y(), rect.width(), rect.height() };
  }
  return Bounds{};
}

} // namespace VGG::layer
