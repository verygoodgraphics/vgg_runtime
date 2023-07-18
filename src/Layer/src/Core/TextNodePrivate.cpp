#include "Core/TextNodePrivate.h"
#include "Core/FontManager.h"
#include "Core/ParagraphParser.h"
#include "Core/TextNode.h"
#include "Core/VGGType.h"
#include "SkiaBackend/SkiaImpl.h"

#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <modules/skparagraph/include/FontCollection.h>

namespace VGG
{

void drawParagraphDebugInfo(DebugCanvas& canvas,
                            const TextParagraph& textParagraph,
                            Paragraph* p,
                            int curX,
                            int curY,
                            int index)
{
  static SkColor colorTable[9] = {
    SK_ColorBLUE,   SK_ColorGREEN,  SK_ColorRED,  SK_ColorCYAN,   SK_ColorMAGENTA,
    SK_ColorYELLOW, SK_ColorDKGRAY, SK_ColorGRAY, SK_ColorLTGRAY,
  };
  canvas.get()->save();
  canvas.get()->translate(curX, curY);
  auto rects = p->getRectsForRange(0, 10000, RectHeightStyle::kMax, RectWidthStyle::kMax);
  auto h = p->getHeight();
  auto mw = p->getMaxWidth();
  auto iw = p->getMaxIntrinsicWidth();
  SkPaint pen;
  SkColor color = colorTable[index % 9];
  pen.setColor(SkColorSetA(color, 0x11));
  canvas.get()->drawRect(SkRect{ 0, 0, mw, h }, pen);
  canvas.drawRects(color, rects);
  canvas.get()->restore();
}

int calcWhitespace(int count,
                   int fontSize,
                   const std::vector<SkString>& fontFamilies,
                   sk_sp<FontCollection> fontCollection)
{
  // cassert(count < 40);
  ParagraphStyle style;
  style.setEllipsis(u"...");
  style.setTextAlign(TextAlign::kLeft);
  style.setMaxLines(1);
  skia::textlayout::TextStyle txtStyle;
  txtStyle.setFontFamilies(fontFamilies);
  txtStyle.setFontSize(fontSize);
  style.setTextAlign(TextAlign::kLeft);
  style.setTextStyle(txtStyle);
  auto builder = ParagraphBuilder::make(style, fontCollection);
  char tabs[40];
  memset(tabs, '\t', count);
  tabs[count] = '\0';
  builder->addText(tabs);
  auto p = builder->Build();
  p->layout(1000);
  const auto rects = p->getRectsForRange(0, count, RectHeightStyle::kMax, RectWidthStyle::kMax);
  if (rects.empty())
    return 0;
  return rects[0].rect.width();
}
sktxt::ParagraphStyle createParagraphStyle(const ParagraphAttr& attr)
{
  ParagraphStyle style;
  style.setEllipsis(u"...");
  style.setTextAlign(toSkTextAlign(attr.horiAlign));
  style.setMaxLines(1000);
  return style;
}

sktxt::TextStyle createTextStyle(const TextAttr& attr, FontCollection* font)
{
  sktxt::TextStyle style;
  SkColor color = attr.color;
  style.setColor(color);
  style.setDecorationColor(color);
  // font->findTypefaces(attr.fontName);
  style.setFontFamilies({ SkString(attr.fontName), SkString("PingFang SC"), SkString("Apple Color Emoji") });
  style.setFontSize(attr.size);
  style.setLetterSpacing(attr.letterSpacing);
  style.setBaselineShift(attr.baselineShift);
  style.setFontStyle(toSkFontStyle(attr.subFamilyName));
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

void TextParagraphCache::onBegin()
{
  paragraph.clear();
}
void TextParagraphCache::onEnd()
{
  set(D_ALL);
}
void TextParagraphCache::onParagraphBegin(int paraIndex,
                                          int order,
                                          const ParagraphAttr& paragraAttr)
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
  newParagraph = true;
  paraAttr = paragraAttr;
}

void TextParagraphCache::onParagraphEnd(int paraIndex, const TextView& textView)
{
  assert(!paragraph.empty());
  auto& p = paragraph.back();
  p.Utf8TextView = textView;
  if (newParagraph)
  {
    if (textView.Text.empty())
      paragraph.pop_back();
    newParagraph = false;
  }
  // std::cout << "onParagraphEnd: " << paraIndex << ", [" << textView.Text << "]\n";
}

void TextParagraphCache::onTextStyle(int paraIndex,
                                     int styleIndex,
                                     const TextView& textView,
                                     const TextAttr& textAttr)
{
  assert(!paragraph.empty());
  auto& p = paragraph.back();
  if (newParagraph)
  {
    fontCollection = FontManager::instance().fontCollection("default");
    p.builder = skia::textlayout::ParagraphBuilder::make(
      createParagraphStyle(ParagraphAttr{ paraAttr.type, textAttr.horzAlignment }),
      fontCollection);
    newParagraph = false;
  }
  p.builder->pushStyle(createTextStyle(textAttr, fontCollection.get()));
  p.builder->addText(textView.Text.data(), textView.Text.size());
  p.builder->pop();
  // std::cout << "Style Text: " << styleIndex << ", [" << textView.Text << "]\n";
}
} // namespace VGG
