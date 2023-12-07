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
#include "ParagraphPainter.hpp"
#include "Layer/AttrSerde.hpp"
#include "Painter.hpp"
#include "Utility/Log.hpp"

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
      for (const auto p : it->second)
      {
        m_canvas->drawTextBlob(blob, x, y, p);
      }
    }
    else
    {
      assert(m_paragraph);
      const auto& style = m_paragraph->textStyles();
      assert(!style.empty());
      const auto fills = style[styleID].fills;
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
          fillPen.setStyle(SkPaint::kFill_Style);
          fillPen.setAntiAlias(true);
          if (f.fillType == FT_Color)
          {
            fillPen.setColor(f.color);
            const auto currentAlpha = fillPen.getAlphaf();
            fillPen.setAlphaf(currentAlpha * f.contextSettings.opacity);
          }
          else if (f.fillType == FT_Gradient)
          {
            assert(f.gradient.has_value());
            auto gradientShader = getGradientShader(f.gradient.value(), m_paragraph->bound());
            fillPen.setShader(gradientShader);
            fillPen.setAlphaf(f.contextSettings.opacity);
          }
          else if (f.fillType == FT_Pattern)
          {
            assert(f.pattern.has_value());
            auto shader = makePatternShader(m_paragraph->bound(), f.pattern.value());
            fillPen.setShader(shader);
            fillPen.setAlphaf(f.contextSettings.opacity);
          }
          paints.push_back(fillPen);
        }
        for (const auto p : paints)
        {
          m_canvas->drawTextBlob(blob, x, y, p);
        }
        m_cache[styleID] = std::move(paints);
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

} // namespace VGG::layer
