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
#include "ParagraphLayout.hpp"

#include "Layer/Core/VType.hpp"
#include "VSkia.hpp"

#include <core/SkFontStyle.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>

using namespace skia::textlayout;
namespace
{

ParagraphStyle createParagraphStyle(const layer::ParagraphAttr& attr)
{
  ParagraphStyle style;
  style.setEllipsis(u"...");
  style.setTextAlign(toSkTextAlign(attr.horiAlign));
  style.setMaxLines(1000);
  return style;
}

std::vector<std::string> split(const std::string& s, char seperator)
{
  std::vector<std::string> output;
  std::string::size_type   prevPos = 0, pos = 0;
  while ((pos = s.find(seperator, pos)) != std::string::npos)
  {
    std::string substring(s.substr(prevPos, pos - prevPos));
    output.push_back(substring);
    prevPos = ++pos;
  }
  output.push_back(s.substr(prevPos, pos - prevPos)); // Last word
  return output;
}

template<typename F>
TextStyle createTextStyle(const TextStyleAttr& attr, VGGFontCollection* font, F&& fun)
{
  TextStyle style;
  if (!attr.fills.empty())
  {
    auto painterID = fun();
    style.setForegroundPaintID(painterID);
    SkColor c = attr.fills[0].color;
    style.setDecorationColor(c);
  }
  else
  {
    SkColor color = SK_ColorBLACK;
    style.setColor(color);
    style.setDecorationColor(color);
  }

  std::string fontName;

  auto resolveFromFallback = [&fontName, &attr](const std::vector<std::string>& candidates)
  {
    for (const auto& candidate : candidates)
    {
      if (const auto components = split(candidate, '-'); !components.empty())
      {
        fontName = components[0];
        break;
      }
    }
  };

  if (const auto components = split(attr.fontName, '-'); !components.empty())
  {
    fontName = components[0];
    auto matched = font->fuzzyMatch(fontName);
    if (matched)
    {
      INFO(
        "Font [%s] matches real name [%s][%f]",
        fontName.c_str(),
        matched->first.c_str(),
        matched->second);
      fontName = std::string(matched->first.c_str());

      // When font name is provided, we match the real name first.
      // If the score is lower than a threshold, we choose the
      // fallback font rather pick it fuzzily.
      constexpr float THRESHOLD = 70.f;
      if (const auto& fallbackFonts = font->fallbackFonts();
          !fallbackFonts.empty() && matched->second < THRESHOLD)
      {
        resolveFromFallback(fallbackFonts);
      }
    }
    else
    {
      DEBUG("No font in font manager");
    }
  }
  else if (const auto& fallbackFonts = font->fallbackFonts(); !fallbackFonts.empty())
  {
    resolveFromFallback(fallbackFonts);
  }
  if (fontName.empty())
  {
    // the worst case
    fontName = "FiraSans";
  }
  DEBUG("Given [%s], [%s] is choosed finally", attr.fontName.c_str(), fontName.c_str());
  std::vector<SkString> fontFamilies;
  fontFamilies.push_back(SkString(fontName));
  if (const auto& fallbackFonts = font->fallbackFonts(); !fallbackFonts.empty())
  {
    for (const auto& f : fallbackFonts)
    {
      if (f.rfind(fontName, 0) != 0) // ignore the added fontName
      {
        fontFamilies.push_back(SkString(f));
      }
    }
  }

  style.setFontFamilies(fontFamilies);
  style.setFontStyle(toSkFontStyle(attr));
  style.setFontSize(attr.size);
  style.setLetterSpacing(attr.letterSpacing);
  style.setBaselineShift(attr.baselineShift);
  if (attr.lineThrough)
  {
    style.setDecoration(skia::textlayout::kLineThrough);
  }
  if (attr.underline != UT_None)
  {
    style.setDecoration(skia::textlayout::kUnderline);
    if (attr.underline == UT_Single)
    {
      style.setDecorationStyle(TextDecorationStyle::kSolid);
    }
    else if (attr.underline == UT_Double)
    {
      style.setDecorationStyle(TextDecorationStyle::kDouble);
    }
  }
  return style;
}
} // namespace
namespace VGG::layer
{

void drawParagraphDebugInfo(
  DebugCanvas&         canvas,
  const TextParagraph& textParagraph,
  Paragraph*           p,
  int                  curX,
  int                  curY,
  int                  index)
{
  // static SkColor s_colorTable[9] = {
  //   SK_ColorBLUE,   SK_ColorGREEN,  SK_ColorRED,  SK_ColorCYAN,   SK_ColorMAGENTA,
  //   SK_ColorYELLOW, SK_ColorDKGRAY, SK_ColorGRAY, SK_ColorLTGRAY,
  // };
  canvas.get()->save();
  canvas.get()->translate(curX, curY);
  auto    h = p->getHeight();
  auto    mw = p->getMaxWidth();
  SkPaint pen;
  // SkColor color = s_colorTable[index % 9];
  pen.setColor(SkColorSetA(SK_ColorBLUE, 0x11));

  canvas.get()->drawRect(SkRect{ 0, 0, mw, h }, pen); // draw the paragraph background

  auto rects = p->getRectsForRange(0, 10000, RectHeightStyle::kMax, RectWidthStyle::kMax);
  canvas.drawRects(SkColorSetA(SK_ColorGREEN, 0x11), rects);

  rects = p->getRectsForPlaceholders();
  canvas.drawRects(SK_ColorRED, rects);

  // p->visit(
  //   [&](int lineNumber, const Paragraph::VisitorInfo* info)
  //   {
  //     if (!info)
  //       return;
  //     SkPaint paint;
  //
  //     //
  //     // paint.setColor(SK_ColorMAGENTA);
  //     // paint.setStrokeWidth(5);
  //     // canvas.get()->drawPoint(info->origin, paint);
  //     //
  //     // paint.setStrokeWidth(1);
  //     // paint.setColor(SK_ColorRED);
  //     // SkPoint to = { info->origin.x() + info->advanceX, info->origin.y() };
  //     // canvas.get()->drawLine(info->origin, to, paint);
  //     // paint.setStrokeWidth(5);
  //     // canvas.get()->drawPoint(to, paint);
  //     //
  //
  //     for (int i = 0; i < info->count; i++)
  //     {
  //       paint.setStrokeWidth(info->font.getSize() / 20);
  //       paint.setColor(SK_ColorBLUE);
  //       SkPoint pos = info->origin + info->positions[i];
  //       canvas.get()->drawPoint(pos, paint);
  //     }
  //   });

  std::vector<LineMetrics> lineMetrics;
  p->getLineMetrics(lineMetrics);

  // float   offsetY = 0;
  SkPaint paint;
  paint.setColor(SK_ColorRED);

  static float s_intervals[2] = { 5, 5 };
  auto         effect = SkDashPathEffect::Make(s_intervals, 2, 25);
  for (const auto& m : lineMetrics)
  {
    // offsetY += m.fHeight;
    // baseline
    canvas.drawHorizontalLine(m.fLeft, m.fWidth + m.fLeft, m.fBaseline, SK_ColorRED, effect);
    // ascent
    canvas
      .drawHorizontalLine(m.fLeft, m.fWidth + m.fLeft, m.fBaseline - m.fAscent, SK_ColorMAGENTA, 0);
    // descent
    canvas
      .drawHorizontalLine(m.fLeft, m.fWidth + m.fLeft, m.fBaseline + m.fDescent, SK_ColorBLUE, 0);

    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    for (auto i = m.fStartIndex; i < m.fEndIndex; i++)
    {
      auto a = p->getRectsForRange(i, i + 1, RectHeightStyle::kStrut, RectWidthStyle::kMax);
      if (a.empty() == false)
        canvas.get()->drawRect(a[0].rect, paint); // draw the paragraph background
    }
  }

  canvas.get()->restore();
}
void RichTextBlock::onBegin()
{
  paragraph.clear();
}
void RichTextBlock::onEnd()
{
}
void RichTextBlock::onParagraphBegin(
  int                  paraIndex,
  int                  order,
  const ParagraphAttr& paragraAttr,
  void*                userData)
{
  paragraph.emplace_back();
  auto& p = paragraph.back();
  if (paragraAttr.type.lineType != TLT_Plain)
  {
    p.level = paragraAttr.type.level;
  }
  else
  {
    p.level = 0;
  }
  m_newParagraph = true;
  m_paraAttr = paragraAttr;
}

void RichTextBlock::onParagraphEnd(int paraIndex, const TextView& textView, void* userData)
{
  assert(!paragraph.empty());
  auto& p = paragraph.back();
  p.utf8TextView = textView;
  if (m_newParagraph)
  {
    if (textView.text.empty())
      paragraph.pop_back();
    m_newParagraph = false;
  }
}

void RichTextBlock::onTextStyle(
  int                  paraIndex,
  int                  styleIndex,
  const TextView&      textView,
  const TextStyleAttr& textAttr,
  void*                userData)
{
  assert(!paragraph.empty());
  auto& p = paragraph.back();
  if (m_newParagraph)
  {
    ETextHorizontalAlignment align;
    std::visit(
      Overloaded{ [&](const TextLayoutAutoHeight& l) { align = textAttr.horzAlignment; },
                  [&](const TextLayoutAutoWidth& l) { align = HA_Left; },
                  [&](const TextLayoutFixed& l) { align = textAttr.horzAlignment; } },
      *(TextLayoutMode*)userData);
    p.builder = skia::textlayout::ParagraphBuilder::make(
      createParagraphStyle(ParagraphAttr{ m_paraAttr.type, align }),
      m_fontCollection);
    m_newParagraph = false;
  }

  auto style =
    createTextStyle(textAttr, m_fontCollection.get(), [&]() { return styleIndex; });
  p.builder->pushStyle(style);
  p.builder->addText(textView.text.data(), textView.text.size());
  p.builder->pop();
}

bool RichTextBlock::ensureBuild(TextLayoutMode mode)
{
  bool ok = false;
  if (m_state <= EMPTY)
    return ok;
  if (m_state < PARSE)
  {
    ParagraphParser p;
    p.parse(*this, m_utf8Text, m_textStyle, m_lineStyle, &mode);
    m_state = PARSE;
  }
  if (!paragraph.empty() && m_state < BUILT)
  {
    paragraphCache.clear();
    paragraphCache.reserve(paragraph.size());
    for (auto& d : paragraph)
    {
      assert(d.builder);
      paragraphCache.push_back(d.build());
    }
    m_state = BUILT;
  }
  if (m_state >= BUILT)
  {
    ok = true;
  }
  return ok;
}

Bound RichTextBlock::internalLayout(const Bound& bound, ETextLayoutMode mode)
{
  Bound      newBound = bound;
  const auto layoutWidth = bound.width();
  float      newWidth = bound.width();
  float      newHeight = bound.height();
  for (std::size_t i = 0; i < paragraphCache.size(); i++)
  {
    // auto paragraph = d.builder->Build();
    const auto&   d = paragraph[i];
    const auto&   paragraph = paragraphCache[i].paragraph;
    SkFontMetrics metrics;
    d.builder->getParagraphStyle().getTextStyle().getFontMetrics(&metrics);
    const auto curX = metrics.fAvgCharWidth * d.level;
    paragraphCache[i].offsetX = curX;
    if (mode == ETextLayoutMode::TL_WidthAuto)
    {
      constexpr float MAX_WIDTH = 100000;
      paragraph->layout(MAX_WIDTH);
    }
    else
    {
      paragraph->layout(layoutWidth - curX);
    }
    newWidth = std::max((float)paragraph->getLongestLine(), newWidth);
    auto lastLine = paragraph->lineNumber();
    if (lastLine < 1)
      continue;
    assert(!d.utf8TextView.text.empty());
    auto c = d.utf8TextView.text.back();
    if (c == '\n' && i < paragraphCache.size() - 1)
    {
      // It not a neat design because list item must be rendered seperately.
      // The line height of newline at each end of paragraph except for the last need to be dealt
      // specially
      LineMetrics lineMetric;
      paragraph->getLineMetricsAt(lastLine - 1, &lineMetric);
      newHeight += paragraph->getHeight() - lineMetric.fHeight;
    }
    else
    {
      newHeight += paragraph->getHeight();
    }
  }
  if (mode == ETextLayoutMode::TL_HeightAuto)
  {
    newBound.setHeight(newHeight);
  }
  if (mode == ETextLayoutMode::TL_WidthAuto)
  {
    newBound.setHeight(newHeight);
    newBound.setWidth(newWidth);
  }
  return newBound;
}

} // namespace VGG::layer
