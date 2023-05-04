#include "TextNode.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

#include <string_view>

namespace VGG
{
std::vector<std::string_view> makeLines(const std::string& text)
{
  std::vector<std::string_view> ls;
  size_t start = 0;
  for (size_t i = 0; i <= text.size(); i++)
  {
    if (i == text.size() || text[i] == '\n')
    {
      ls.push_back(text.substr(start, i - start));
      start = i + 1;
    }
  }
  return ls;
}

void drawFramedText(SkCanvas* canvas,
                    const std::string& text,
                    const Bound2& frame,
                    const TextStyleStub& textStyle)
{
  ASSERT(canvas);

  auto textLines = makeLines(text);
  SkFontMetrics metrics;
  SkFont font = textStyle.getFont();
  font.getMetrics(&metrics);
  double y = std::fabs(metrics.fAscent);
  double df = std::fabs(metrics.fAscent) + std::fabs(metrics.fDescent);
  double dh = textStyle.lineSpace * df;
  double dl = textStyle.paraSpacing;

  if (textStyle.vertAlignment == VA_Center)
  {
    double totalHeight = frame.height();
    auto sz = textLines.size();
    double textHeight = std::max(0.0, sz * dh + sz * dl - dl);
    double dy = (totalHeight - textHeight) / 2;
    y += dy;
  }
  else if (textStyle.vertAlignment == VA_Bottom)
  {
    double totalHeight = frame.height();
    auto sz = textLines.size();
    double textHeight = std::max(0.0, sz * dh + sz * dl - dl);
    double dy = totalHeight - textHeight;
    y += dy;
  }

  SkPaint pen = textStyle.getPaint();
  for (auto line : textLines)
  {
    if (line.empty())
    {
      y += (dh + dl);
      continue;
    }

    double tw = 0;
    std::vector<SkScalar> xs;
    for (size_t i = 0; i < line.size(); i++)
    {
      xs.push_back(tw);
      // auto c = TextCodecs::conv.to_bytes(line[i]);
      auto cw = font.measureText(line.data(), line.size(), SkTextEncoding::kUTF8);
      tw += (cw + textStyle.letterSpacing);
    }
    tw -= textStyle.letterSpacing;

    auto tb = SkTextBlob::MakeFromPosTextH(line.data(),
                                           line.size(),
                                           xs.data(),
                                           (dh - df) / 2,
                                           font,
                                           SkTextEncoding::kUTF8);

    double x = 0;
    if (textStyle.horzAlignment == HA_Center)
    {
      x += (frame.width() - tw) / 2;
    }
    else if (textStyle.horzAlignment == HA_Right)
    {
      x += (frame.width() - tw);
    }
    canvas->drawTextBlob(tb, x, y, pen);

    if (textStyle.lineThrough)
    {
      SkPaint p = pen;
      p.setStrokeWidth(metrics.fStrikeoutThickness);
      canvas->drawLine(x,
                       y + metrics.fStrikeoutPosition,
                       x + tw,
                       y + metrics.fStrikeoutPosition,
                       p);
    }
    if (textStyle.underline)
    {
      SkPaint p = pen;
      p.setStrokeWidth(metrics.fUnderlineThickness);
      canvas->drawLine(x,
                       y + metrics.fUnderlinePosition,
                       x + tw,
                       y + metrics.fUnderlinePosition,
                       p);
    }

    y += (dh + dl);
  }
}

TextNode::TextNode(const std::string& name, const std::string& text)
  : PaintNode(name, VGG_TEXT)
  , text(text)
{
}

void TextNode::Paint(SkCanvas* canvas)
{
  if (styles.empty() == false)
  {
    drawFramedText(canvas, text, bound, styles[0]);
  }
}

} // namespace VGG
