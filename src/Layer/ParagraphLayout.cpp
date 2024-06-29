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
#include "ParagraphLayout.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/FontManager.hpp"
#include "Layer/SkiaFontManagerProxy.hpp"
#include "Layer/VSkFontMgr.hpp"
#include "VSkia.hpp"

#include <algorithm>
#include <core/SkColor.h>
#include <core/SkFontArguments.h>
#include <core/SkFontStyle.h>
#include <core/SkTypes.h>
#include <effects/SkDashPathEffect.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <optional>

using namespace skia::textlayout;
namespace
{
using namespace VGG::layer;
using namespace VGG;
constexpr SkScalar WEIGHT[] = { SkFontStyle::kInvisible_Weight,  SkFontStyle::kThin_Weight,
                                SkFontStyle::kExtraLight_Weight, SkFontStyle::kLight_Weight,
                                SkFontStyle::kNormal_Weight,     SkFontStyle::kMedium_Weight,
                                SkFontStyle::kSemiBold_Weight,   SkFontStyle::kExtraBold_Weight,
                                SkFontStyle::kBlack_Weight,      SkFontStyle::kExtraBlack_Weight };

SkFontStyle::Weight skFontStyleWeightForWeightAxisValue(SkScalar weight)
{
  const int w = SkScalarRoundToInt(SkScalarInterpFunc(weight, WEIGHT, WEIGHT, 10));
  return static_cast<SkFontStyle::Weight>(w);
}

float findWeight(std::string_view key)
{
  static std::vector<std::pair<std::string_view, float>> s_fontWeightMap = {
    { "extrablack", SkFontStyle::kExtraBlack_Weight },
    { "black", SkFontStyle::kBlack_Weight },
    { "extrabold", SkFontStyle::kExtraBold_Weight },
    { "semibold", SkFontStyle::kSemiBold_Weight },
    { "bold", SkFontStyle::kBold_Weight },
    { "medium", SkFontStyle::kMedium_Weight },
    { "extralight", SkFontStyle::kExtraLight_Weight },
    { "light", SkFontStyle::kLight_Weight },
    { "thin", SkFontStyle::kThin_Weight },
    { "regular", SkFontStyle::kNormal_Weight }
  };
  for (const auto& [k, v] : s_fontWeightMap)
  {
    if (key.find(k) != std::string::npos)
    {
      return v;
    }
  }
  return (float)SkFontStyle::kNormal_Weight;
}

float findWidth(std::string_view key)
{
  static std::vector<std::pair<std::string_view, float>> s_fontWidthMap = {
    { "semicondensed", 87.5 }, { "semiexpanded", 112.5 },  { "extraexpanded", 150 },
    { "ultraexpanded", 200 },  { "extracondensed", 62.5 }, { "ultracondensed", 50 },
    { "expanded", 125 },       { "condensed", 75 }
  };
  for (const auto& [k, v] : s_fontWidthMap)
  {
    if (key.find(k) != std::string::npos)
    {
      return v;
    }
  }
  return 100.f;
}

std::pair<SkFontStyle, std::vector<Font::Axis>> toSkFontStyle(const Font& font)
{
  std::string subFamilyName;
  // std::remove_copy(
  //   font.subFamilyName.begin(),
  //   font.subFamilyName.end(),
  //   std::back_inserter(subFamilyName),
  //   ' ');

  std::transform(
    font.subFamilyName.begin(),
    font.subFamilyName.end(),
    std::back_inserter(subFamilyName),
    [](char c) { return std::tolower(c); });

  SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
  if (
    subFamilyName.find("italic") != std::string::npos ||
    font.fontName.find("italic") != std::string::npos)
    slant = SkFontStyle::kItalic_Slant;

  static struct
  {
    using Func = float (*)(std::string_view);
    uint32_t tag;
    Func     find;
  } s_axisInSubFamilyName[] = { { Font::wght, findWeight }, { Font::wdth, findWidth } };

  std::vector<Font::Axis> finalAxis;
  finalAxis.reserve(2 + font.axis.size());
  for (const auto& [t, f] : s_axisInSubFamilyName)
  {
    if (!Font::axisValue(font.axis, t))
      finalAxis.emplace_back(t, f(subFamilyName));
  }
  finalAxis.insert(finalAxis.end(), font.axis.begin(), font.axis.end());

  SkFontStyle::Weight weight =
    skFontStyleWeightForWeightAxisValue(Font::axisValue(finalAxis, Font::wght).value());
  SkFontStyle::Width width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(
    Font::axisValue(finalAxis, Font::wdth).value());

  return { { weight, width, slant }, std::move(finalAxis) };
}

ParagraphStyle createParagraphStyle(const layer::ParagraphAttr& attr)
{
  ParagraphStyle style;
  style.setEllipsis(u"...");
  style.setTextAlign(toSkTextAlign(attr.horiAlign));
  style.setMaxLines(1000);
  return style;
}

template<typename F>
TextStyle createTextStyle(const TextStyleAttr& attr, F&& fun)
{
  TextStyle style;
  if (!attr.fills.empty())
  {
    auto painterID = fun();
    style.setForegroundPaintID(painterID);
    std::visit(
      Overloaded{ [&](const Gradient& g) { WARN("Don't support gradient decoration"); },
                  [&](const Color& c) { style.setDecorationColor(c); },
                  [&](const Pattern& p) { WARN("Don't support pattern decoration"); } },
      attr.fills[0].type);
  }
  else
  {
    SkColor color = SK_ColorBLACK;
    style.setColor(color);
    style.setDecorationColor(color);
  }

  std::string fontName = FontManager::getFontMananger().matchFontName(attr.font.fontName);

  std::vector<SkString> fontFamilies;
  fontFamilies.push_back(SkString(fontName));
  if (const auto& fallbackFonts = FontManager::getFontMananger().fallbackFonts();
      !fallbackFonts.empty())
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
  auto [fontStyle, newAxis] = toSkFontStyle(attr.font);
  auto fontMgr = SkiaFontManagerProxy(FontManager::getFontMananger()).skFontMgr();
  ASSERT(fontMgr);
  auto ft = fontMgr->matchFamilyStyle(fontName.c_str(), fontStyle);
  if (ft && ft->fontStyle() == fontStyle)
  {
    style.setFontStyle(fontStyle);
    std::vector<SkFontArguments::VariationPosition::Coordinate> coords;
    coords.reserve(newAxis.size());
    for (const auto& axis : newAxis)
    {
      if (Font::wght == axis.name)
        continue; // skip wght axis setting
      coords.push_back({ axis.name, axis.value });
    }
    style.setFontArguments(
      SkFontArguments().setVariationDesignPosition({ coords.data(), (int)coords.size() }));
  }
  else
  {
    std::vector<SkFontArguments::VariationPosition::Coordinate> coords;
    coords.reserve(newAxis.size());
    for (const auto& axis : newAxis)
      coords.push_back({ axis.name, axis.value });
    style.setFontArguments(
      SkFontArguments().setVariationDesignPosition({ coords.data(), (int)coords.size() }));
  }

  style.setFontSize(attr.font.size);
  style.setLetterSpacing(attr.letterSpacing);
  style.setBaselineShift(attr.baselineShift);
  if (attr.lineHeight)
  {
    style.setHeight(*attr.lineHeight);
    style.setHeightOverride(true);
  }

  if (attr.lineThrough)
  {
    style.setDecoration(skia::textlayout::kLineThrough);
  }
  if (attr.underline != UT_NONE)
  {
    style.setDecoration(skia::textlayout::kUnderline);
    if (attr.underline == UT_SINGLE)
    {
      style.setDecorationStyle(TextDecorationStyle::kSolid);
    }
    else if (attr.underline == UT_DOUBLE)
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
  if (paragraAttr.type.lineType != TLT_PLAIN)
  {
    p.level = paragraAttr.type.level;
  }
  else
  {
    p.level = 0;
  }
  p.builder = skia::textlayout::ParagraphBuilder::make(
    createParagraphStyle(ParagraphAttr{ m_paraAttr.type, paragraAttr.horiAlign }),
    m_fontCollection);
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
    m_newParagraph = false;
  }

  auto style = createTextStyle(textAttr, [&]() { return styleIndex; });
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
    ParagraphParser p(true);
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

std::pair<Bounds, float> RichTextBlock::internalLayout(const Bounds& bounds, ETextLayoutMode mode)
{
  Bounds     newBounds = bounds;
  const auto layoutWidth = bounds.width();
  float      newWidth = bounds.width();
  float      newHeight = 0.f;

  float maxWidth = 0.f;
  if (mode == TL_AUTOWIDTH)
  {
    for (std::size_t i = 0; i < paragraphCache.size(); i++)
    {
      constexpr float MAX_WIDTH = 100000;
      auto            oldStyle = paragraph[i].builder->getParagraphStyle().getTextAlign();
      paragraphCache[i].paragraph->updateTextAlign(TextAlign::kLeft);
      paragraphCache[i].paragraph->layout(MAX_WIDTH);
      auto width = paragraphCache[i].paragraph->getLongestLine();
      if (maxWidth < width)
      {
        maxWidth = width;
      }
      paragraphCache[i].paragraph->updateTextAlign(oldStyle);
    }
  }
  for (std::size_t i = 0; i < paragraphCache.size(); i++)
  {
    // auto paragraph = d.builder->Build();
    const auto&   d = paragraph[i];
    const auto&   paragraph = paragraphCache[i].paragraph;
    SkFontMetrics metrics;
    d.builder->getParagraphStyle().getTextStyle().getFontMetrics(&metrics);
    const auto curX = metrics.fAvgCharWidth * d.level;
    paragraphCache[i].offsetX = curX;
    if (mode == ETextLayoutMode::TL_AUTOWIDTH)
    {
      paragraph->layout(maxWidth + 1);
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
  switch (mode)
  {
    case TL_AUTOHEIGHT:
      newBounds.setHeight(newHeight);
      break;
    case TL_AUTOWIDTH:
      newBounds.setHeight(newHeight);
      newBounds.setWidth(newWidth);
      break;
    case TL_FIXED:
      break;
  }
  return { newBounds, newHeight };
}

} // namespace VGG::layer
