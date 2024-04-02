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
#include "ShapeItem.hpp"
#include "Effects.hpp"
#include "FillEffects.hpp"

namespace VGG::layer
{

void ShapeItem::render(Renderer* renderer)
{
  if (auto shapeAttr = this->shape(); shapeAttr)
  {
    auto parent = m_objectAttribute.lock();
    if (auto shape = shapeAttr->getShape(); !shape.isEmpty() && parent)
    {
      ObjectRecorder rec;
      SkRect         objectBounds = shape.bounds();
      auto           recorder = rec.beginRecording(objectBounds, SkMatrix::I());
      auto           fillBounds = shape.bounds();
      FillEffect     fillEffect(parent->getFillStyle(), fillBounds, 0, 0);
      fillEffect.render(renderer, shape);
      const auto borderBounds =
        VGG::layer::drawBorder(recorder, shape, shape.bounds(), parent->getBorderStyle(), 0);
      objectBounds.join(fillBounds);
      objectBounds.join(borderBounds);
      auto mat = SkMatrix::Translate(objectBounds.x(), objectBounds.y());
      m_objectShader = rec.finishRecording(objectBounds, &mat);
      m_objectShader->render(renderer);
    }
  }
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

Bounds ShapeItem::onRevalidate()
{
  ASSERT(m_shapeAttr);
  m_shapeAttr->revalidate();
  auto parent = m_objectAttribute.lock();
  if (parent)
  {
    if (const auto& shape = m_shapeAttr->getShape(); !shape.isEmpty())
    {
      auto shapeBounds = shape.bounds();
      auto [bounds, paint] = revalidateObjectBounds(parent->getBorderStyle(), shapeBounds);
      ObjectRecorder rec;
      auto           recorder = rec.beginRecording(bounds, SkMatrix::I());
      SkPaint        fillPaint;
      fillPaint.setAntiAlias(true);
      fillPaint.setStyle(SkPaint::kFill_Style);
      fillPaint.setAlphaf(1.0f);
      shape.draw(recorder->canvas(), fillPaint);
      if (auto strokePen = paint; strokePen)
      {
        strokePen->setAlphaf(1.0f);
        shape.draw(recorder->canvas(), *strokePen);
      }
      auto mat = SkMatrix::Translate(bounds.x(), bounds.y());
      auto object = rec.finishRecording(bounds, &mat);
      m_maskFilter = object.asImageFilter();
      m_effectBounds = Bounds{ bounds.x(), bounds.y(), bounds.width(), bounds.height() };
      return Bounds{ shapeBounds.x(), shapeBounds.y(), shapeBounds.width(), shapeBounds.height() };
    }
  }
  return Bounds();
}

} // namespace VGG::layer
