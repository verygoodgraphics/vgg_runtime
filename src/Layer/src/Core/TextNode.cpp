#include "Core/TextNode.h"
#include "Core/FontManager.h"
#include "Core/Node.hpp"
#include "Core/VGGType.h"
#include "Core/VGGUtils.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/effects/SkRuntimeEffect.h"
#include "core/SkTextBlob.h"

#include "Core/TextNodePrivate.h"

#include <memory>
#include <modules/skparagraph/include/FontCollection.h>
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
      ls.emplace_back(text.data() + start, i - start);
      start = i + 1;
    }
  }
  return ls;
}

const char* next_utf8_char(const unsigned char* p_begin)
{
  if (*p_begin >> 7 == 0)
  {
    ++p_begin;
  }
  else if (*p_begin >> 5 == 6 && p_begin[1] >> 6 == 2)
  {
    p_begin += 2;
  }
  else if (*p_begin >> 4 == 0x0E && p_begin[1] >> 6 == 2 && p_begin[2] >> 6 == 2)
  {
    p_begin += 3;
  }
  else if (*p_begin >> 3 == 0x1E && p_begin[1] >> 6 == 2 && p_begin[2] >> 6 == 2 &&
           p_begin[3] >> 6 == 2)
  {
    p_begin += 4;
  }
  else
  {
    assert(false);
  }
  return (const char*)p_begin;
}

void drawText(SkCanvas* canvas,
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
  double dh = 1.0 * df;
  // double dh = textStyle.lineSpace * df;
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
    auto left_bytes = line.size();
    const auto end = line.data() + left_bytes;
    auto cur = line.data();
    while (cur != end)
    {
      auto next = next_utf8_char((const unsigned char*)cur);
      xs.push_back(tw);
      auto cw = font.measureText(cur, next - cur, SkTextEncoding::kUTF8);
      tw += (cw + textStyle.letterSpacing);
      cur = next;
    }

    // for (size_t i = 0; i < line.size(); i++)
    // {
    //   xs.push_back(tw);
    //   // auto c = TextCodecs::conv.to_bytes(line[i]);
    //   auto cw = font.measureText(&line[i], 1, SkTextEncoding::kUTF8);
    //   tw += (cw + textStyle.letterSpacing);
    // }
    //
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

    // {
    //   SkTextBlobBuilder textBlobBuilder;
    //   SkFont font;
    //   font.setSize(50);
    //   const SkTextBlobBuilder::RunBuffer& run = textBlobBuilder.allocRun(font, 1, 0, 0);
    //   run.glyphs[0] = 20;
    //   sk_sp<const SkTextBlob> blob = textBlobBuilder.make();
    //   SkPaint paint;
    //   paint.setColor(SK_ColorBLUE);
    //   canvas->drawTextBlob(blob.get(), 0, 0, paint);
    // }

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

TextNode::TextNode(const std::string& name)
  : PaintNode(name, VGG_TEXT)
  , d_ptr(new TextNode__pImpl(this))
{
}

void TextNode::setText(const std::string& utf8, const std::vector<TextStyleStub>& styles)
{
  VGG_IMPL(TextNode)
  _->text = utf8;
  _->styles = styles;
}

void TextNode::setTextStyle(size_t position, const TextStyleStub& style)
{
}

void TextNode::setParagraph(const std::string& utf8,
                            const std::vector<TextAttr>& attrs,
                            const std::vector<TextLineAttr>& lineAttr)
{
  VGG_IMPL(TextNode);
  std::vector<ParagraphSet::ParagraphAttr> paraAttrs;
  for (const auto a : lineAttr)
  {
    paraAttrs.emplace_back(ParagraphStyle(), a);
  }
  if (!paraAttrs.empty())
  {
    _->m_paragraphSet = std::make_unique<ParagraphSet>(utf8, attrs, paraAttrs, nullptr);
  }
}

void TextNode::setFrameMode(ETextLayoutMode mode)
{
  VGG_IMPL(TextNode)
  _->mode = mode;
}

void TextNode::paintEvent(SkCanvas* canvas)
{
  VGG_IMPL(TextNode);
  if (_->styles.empty() == false && _->text.empty() == false)
  {
    canvas->save();
    canvas->clipRect(toSkRect(getBound()));
    canvas->scale(1, -1);
    // we need to convert to skia coordinate to render text
    drawText(canvas, _->text, getBound(), _->styles[0]);
    canvas->restore();
  }

  // if (_->m_paragraphSet)
  // {
  //   canvas->save();
  //   canvas->clipRect(toSkRect(getBound()));
  //   canvas->scale(1, -1);
  //   // we need to convert to skia coordinate to render text
  //   _->drawParagraph(canvas);
  //   canvas->restore();
  // }
}

void TextNode::setVerticalAlignment(ETextVerticalAlignment vertAlign)
{
  VGG_IMPL(TextNode);
  _->m_vertAlign = vertAlign;
}

TextNode::~TextNode() = default;

} // namespace VGG
