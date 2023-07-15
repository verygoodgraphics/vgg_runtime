#include "Core/TextNodePrivate.h"
#include "Core/FontManager.h"
#include "Core/ParagraphParser.h"
#include "Core/TextNode.h"
#include "Core/VGGType.h"
#include "SkiaBackend/SkiaImpl.h"

#include <core/SkCanvas.h>
#include <core/SkColor.h>

namespace VGG
{

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
skia::textlayout::ParagraphStyle createParagraphStyle(const ParagraphAttr& attr)
{
  ParagraphStyle style;
  style.setEllipsis(u"...");
  style.setTextAlign(toSkTextAlign(attr.horiAlign));
  style.setMaxLines(1000);
  return style;
}

skia::textlayout::TextStyle createTextStyle(const TextAttr& attr)
{
  skia::textlayout::TextStyle style;
  SkColor color = attr.color;
  style.setColor(color);
  style.setDecorationColor(color);
  style.setFontFamilies({ SkString(attr.fontName) });
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
  // style.setFontStyle();
  return style;
}

void TextParagraphCache::onBegin()
{
  paragraph.clear();
}
void TextParagraphCache::onEnd()
{
  markDirty();
}
void TextParagraphCache::onParagraphBegin(int paraIndex,
                                          int order,
                                          const ParagraphAttr& paragraAttr)
{
  paragraph.emplace_back();
  auto& p = paragraph.back();

  if (paragraAttr.type.lineType != TLT_Plain)
  {
    p.level = paragraAttr.type.level * 20;
  }
  else
  {
    p.level = 0;
  }
  newParagraph = true;
  paraAttr = paragraAttr;
  // p.level = calcWhitespace(paragraAttr.type.level, 14, {}, defaultFontCollection);
  // std::cout << "onParagraphBegin: " << paraIndex << ", " << paragraAttr.type.level << "\n";
}

void TextParagraphCache::onParagraphEnd(int paraIndex, const TextView& textView)
{
  assert(!paragraph.empty());
  auto& p = paragraph.back();
  p.Utf8TextView = textView;
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
    auto defaultFontCollection = FontManager::instance().fontCollection("default");
    p.builder = skia::textlayout::ParagraphBuilder::make(
      createParagraphStyle(ParagraphAttr{ paraAttr.type, textAttr.horzAlignment }),
      defaultFontCollection);
    newParagraph = false;
  }
  p.builder->pushStyle(createTextStyle(textAttr));
  p.builder->addText(textView.Text.data(), textView.Text.size());
  p.builder->pop();
  // std::cout << "Style Text: " << styleIndex << ", [" << textView.Text << "]\n";
}
} // namespace VGG
