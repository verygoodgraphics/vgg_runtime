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
#include "ParagraphPainter.hpp"
#include "Layer/Settings.hpp"
#include "VSkia.hpp"
#include "Effects.hpp"

#include <core/SkColor.h>
#include <core/SkPaint.h>
#include <include/core/SkBlurTypes.h>
#include <include/core/SkMaskFilter.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/effects/SkDiscretePathEffect.h>
#include <variant>

namespace VGG::layer
{

void VParagraphPainter::drawTextBlob(
  const sk_sp<SkTextBlob>& blob,
  SkScalar                 x,
  SkScalar                 y,
  const SkPaintOrID&       paint)
{
  if (auto p = std::get_if<SkPaint>(&paint))
  {
    m_canvas->drawTextBlob(blob, x, y, *p);
  }
  else if (auto p = std::get_if<PaintID>(&paint))
  {
    auto styleID = *p;
    if (auto it = m_cache.find(styleID); it != m_cache.end())
    {
      for (const auto& p : it->second)
      {
        m_canvas->drawTextBlob(blob, x, y, p);
      }
    }
    else
    {
      assert(m_paragraph);
      const auto& style = m_paragraph->textStyles();
      assert(!style.empty());
      const auto& fills = style[styleID].fills;
      if (fills.empty())
      {
        SkPaint defaultPaint;
        defaultPaint.setColor(SK_ColorBLACK);
        defaultPaint.setAntiAlias(true);
        m_canvas->drawTextBlob(blob, x, y, defaultPaint);
      }
      else
      {
        SkPaint fillPen;
        fillPen.setAntiAlias(true);
        std::vector<SkPaint> paints;
        for (const auto& f : fills)
        {
          if (!f.isEnabled)
            continue;
          fillPen.setStyle(SkPaint::kFill_Style);
          fillPen.setAntiAlias(true);
          populateSkPaint(f.type, f.contextSettings, toSkRect(m_paragraph->bounds()), fillPen);
          paints.push_back(fillPen);
        }
        if (!paints.empty())
        {
          for (const auto& p : paints)
          {
            m_canvas->drawTextBlob(blob, x, y, p);
          }
          m_cache[styleID] = std::move(paints);
        }
      }
    }
  }
}

void VParagraphPainter::drawTextShadow(
  const sk_sp<SkTextBlob>& blob,
  SkScalar                 x,
  SkScalar                 y,
  SkColor                  color,
  SkScalar                 blurSigma)
{
  SkPaint paint;
  paint.setColor(color);
  if (blurSigma != 0.0)
  {
    sk_sp<SkMaskFilter> filter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, blurSigma, false);
    paint.setMaskFilter(filter);
  }
  m_canvas->drawTextBlob(blob, x, y, paint);
}

void VParagraphPainter::drawRect(const SkRect& rect, const SkPaintOrID& paint)
{
  SkASSERT(std::holds_alternative<SkPaint>(paint));
  m_canvas->drawRect(rect, std::get<SkPaint>(paint));
}

void VParagraphPainter::drawFilledRect(const SkRect& rect, const DecorationStyle& decorStyle)
{
  SkPaint p(decorStyle.skPaint());
  p.setStroke(false);
  m_canvas->drawRect(rect, p);
}

void VParagraphPainter::drawPath(const SkPath& path, const DecorationStyle& decorStyle)
{
  m_canvas->drawPath(path, decorStyle.skPaint());
}

void VParagraphPainter::drawLine(
  SkScalar               x0,
  SkScalar               y0,
  SkScalar               x1,
  SkScalar               y1,
  const DecorationStyle& decorStyle)
{
  m_canvas->drawLine(x0, y0, x1, y1, decorStyle.skPaint());
}

void VParagraphPainter::clipRect(const SkRect& rect)
{
  m_canvas->clipRect(rect);
}

void VParagraphPainter::translate(SkScalar dx, SkScalar dy)
{
  m_canvas->translate(dx, dy);
}

void VParagraphPainter::save()
{
  m_canvas->save();
}

void VParagraphPainter::restore()
{
  m_canvas->restore();
}
void VParagraphPainter::paintRaw(Renderer* renderer, float x, float y)
{
  float offsetY = y;
  setCanvas(renderer->canvas());
  for (std::size_t i = 0; i < m_paragraph->paragraphCache.size(); i++)
  {
    auto&      p = m_paragraph->paragraphCache[i].paragraph;
    const auto curX = m_paragraph->paragraphCache[i].offsetX + x;
    p->paint(this, m_paragraph->paragraphCache[i].offsetX, offsetY);
    if (getDebugBoundsEnable())
    {
      DebugCanvas debugCanvas(renderer->canvas());
      drawParagraphDebugInfo(debugCanvas, m_paragraph->paragraph[i], p.get(), curX, offsetY, i);
    }
    auto        lastLine = p->lineNumber();
    LineMetrics lineMetric;
    if (lastLine < 1)
      continue;
    p->getLineMetricsAt(lastLine - 1, &lineMetric);
    offsetY += p->getHeight() - lineMetric.fHeight;
  }
}

void VParagraphPainter::paintParagraph(Renderer* renderer)
{
  const auto b = m_paragraph->bounds();
  float      totalHeight = m_paragraph->textHeight();
  float      offsetY = 0.f;
  auto       vertAlign = m_paragraph->getVerticalAlignment();
  switch (vertAlign)
  {
    case VA_BOTTOM:
      offsetY = b.height() - totalHeight;
      break;
    case VA_CENTER:
      offsetY = (b.height() - totalHeight) / 2.f;
      break;
    case VA_TOP:
      offsetY = 0.f;
    default:
      break;
  }
  paintRaw(renderer, 0.f, offsetY);
}

} // namespace VGG::layer
