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
#include <include/core/SkCanvas.h>
#include <include/core/SkPath.h>
#include <modules/skparagraph/include/ParagraphPainter.h>

#include <unordered_map>

namespace VGG
{
namespace layer
{

class VParagraphPainter : public skia::textlayout::ParagraphPainter
{
  const std::vector<TextStyleAttr>* m_textStyle;
  std::unordered_map<int, SkPaint>  m_cache;
  Bound                             m_bound;

public:
  VParagraphPainter(
    SkCanvas*                         canvas,
    const std::vector<TextStyleAttr>* textAttr,
    const Bound&                      bound)
    : m_canvas(canvas)
    , m_textStyle(textAttr)
    , m_bound(bound)
  {
  }

  void setCanvas(SkCanvas* canvas)
  {
    m_canvas = canvas;
  }

  void updateBound(const Bound& bound)
  {
    m_bound = bound;
    m_cache.clear();
  }

  void updateTextStyle(const std::vector<TextStyleAttr>* textStyle)
  {
    m_textStyle = textStyle;
    m_cache.clear();
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
};
} // namespace layer
} // namespace VGG
