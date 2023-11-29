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
#pragma once

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/ParagraphLayout.hpp"
#include "Layer/Renderer.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkPath.h>
#include <modules/skparagraph/include/ParagraphPainter.h>

#include <unordered_map>

namespace VGG
{
namespace layer
{

class VParagraphPainter
  : public skia::textlayout::ParagraphPainter
  , public VNode
{
  std::unordered_map<int, SkPaint>    m_cache;
  std::shared_ptr<TextParagraphCache> m_paragraph;

public:
  VParagraphPainter()
  {
  }

  void setParagraph(std::shared_ptr<TextParagraphCache> paragraph)
  {
    if (m_paragraph == paragraph)
      return;
    if (m_paragraph)
      unobserve(m_paragraph);
    m_paragraph = std::move(paragraph);
    observe(m_paragraph);
  }

  void setCanvas(SkCanvas* canvas)
  {
    m_canvas = canvas;
  }

  void paint(SkiaRenderer* renderer)
  {
    const auto b = m_paragraph->bound();
    setCanvas(renderer->canvas());
    int  totalHeight = m_paragraph->getHeight();
    auto vertAlign = m_paragraph->verticalAlignment();
    int  curY = 0;
    if (vertAlign == ETextVerticalAlignment::VA_Bottom)
    {
      curY = b.height() - totalHeight;
    }
    else if (vertAlign == ETextVerticalAlignment::VA_Center)
    {
      curY = (b.height() - totalHeight) / 2;
    }
    for (int i = 0; i < m_paragraph->paragraphCache.size(); i++)
    {
      auto&      p = m_paragraph->paragraphCache[i].paragraph;
      const auto curX = m_paragraph->paragraphCache[i].offsetX;
      p->paint(this, curX, curY);
      if (renderer->isEnableDrawDebugBound())
      {
        DebugCanvas debugCanvas(renderer->canvas());
        drawParagraphDebugInfo(debugCanvas, m_paragraph->paragraph[i], p.get(), curX, curY, i);
      }
      auto        lastLine = p->lineNumber();
      LineMetrics lineMetric;
      if (lastLine < 1)
        continue;
      p->getLineMetricsAt(lastLine - 1, &lineMetric);
      curY += p->getHeight() - lineMetric.fHeight;
    }
  }

  VParagraphPainter(const VParagraphPainter&) = delete;
  VParagraphPainter& operator=(const VParagraphPainter&) = delete;
  VParagraphPainter(VParagraphPainter&&) = delete;
  VParagraphPainter& operator=(VParagraphPainter&&) = delete;

  void drawTextBlob(const sk_sp<SkTextBlob>& blob, SkScalar x, SkScalar y, const SkPaintOrID& paint)
    override;
  void drawTextShadow(
    const sk_sp<SkTextBlob>& blob,
    SkScalar                 x,
    SkScalar                 y,
    SkColor                  color,
    SkScalar                 blurSigma) override;
  void drawRect(const SkRect& rect, const SkPaintOrID& paint) override;
  void drawFilledRect(const SkRect& rect, const DecorationStyle& decorStyle) override;
  void drawPath(const SkPath& path, const DecorationStyle& decorStyle) override;
  void drawLine(
    SkScalar               x0,
    SkScalar               y0,
    SkScalar               x1,
    SkScalar               y1,
    const DecorationStyle& decorStyle) override;

  void clipRect(const SkRect& rect) override;
  void translate(SkScalar dx, SkScalar dy) override;

  void save() override;
  void restore() override;

private:
  SkCanvas* m_canvas;

protected:
  Bound onRevalidate() override
  {
    auto newBound = m_paragraph->revalidate();
    m_cache.clear();
    return newBound;
  }
};
} // namespace layer
} // namespace VGG
