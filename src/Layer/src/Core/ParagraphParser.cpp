#include "Core/ParagraphParser.h"
#include "Core/VGGType.h"

namespace VGG
{

enum class Reason
{
  Finish,
  Terminate,
  InvalidUTF8,
};
template<typename F>
Reason runOnUtf8(const char* utf8, size_t bytes, F&& f)
{
  const char* cur = utf8;
  const char* next_char = nullptr;
  int char_count = 0;
  int bytes_count = 0;
  auto next_utf8_char = [](unsigned char* p_begin, int& char_count) -> const char*
  {
    if (*p_begin >> 7 == 0)
    {
      char_count = 1;
      ++p_begin;
    }
    else if (*p_begin >> 5 == 6 && p_begin[1] >> 6 == 2)
    {
      p_begin += 2;
      char_count = 1;
    }
    else if (*p_begin >> 4 == 0x0E && p_begin[1] >> 6 == 2 && p_begin[2] >> 6 == 2)
    {
      p_begin += 3;
      char_count = 1;
    }
    else if (*p_begin >> 3 == 0x1E && p_begin[1] >> 6 == 2 && p_begin[2] >> 6 == 2 &&
             p_begin[3] >> 6 == 2)
    {
      p_begin += 4;
      char_count = 2;
    }
    else
    {
      assert(false && "invalid utf8");
      return nullptr;
    }
    return (const char*)p_begin;
  };
  while ((next_char = next_utf8_char((unsigned char*)cur, char_count)))
  {
    bytes_count += (next_char - cur);
    if (bytes_count > bytes)
    {
      return Reason::Finish;
    }
    if (!f(cur, next_char, char_count))
      return Reason::Terminate;
    cur = next_char;
  }
  return Reason::InvalidUTF8;
}
skia::textlayout::ParagraphStyle createParagraphStyle(const ParagraphAttr& attr)
{
  ParagraphStyle style;
  style.setEllipsis(attr.ellipsis);
  style.setTextAlign(TextAlign::kLeft);
  style.setMaxLines(attr.maxLines);
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
int ParagraphParser::calcWhitespace(int count,
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

std::vector<TextParagraph> ParagraphParser::parse(
  const std::string& text,
  const std::vector<TextAttr>& textAttrs,
  const std::vector<ParagraphAttr>& paragraphAttributes,
  sk_sp<FontCollection> fontCollection)
{
  assert(!textAttrs.empty());
  assert(!paragraphAttributes.empty());
  assert(!text.empty());
  reset(text, textAttrs[0].length);
  std::vector<TextParagraph> paragraphs;
  auto builder = skia::textlayout::ParagraphBuilder::make(
    createParagraphStyle(paragraphAttributes[paragraphAttrIndex]),
    fontCollection);
  assert(builder);

  int paragraphCharCount = 0;

  int intendationOffset = 0;
  int currentLevel = 0;

  auto reason =
    runOnUtf8(text.c_str(),
              text.size(),
              [&, this](const char* begin, const char* end, int charCount)
              {
                this->length += charCount;
                paragraphCharCount += charCount;
                unsigned char newLine = *begin;
                bool skip = false;
                const auto pa = paragraphAttributes[paragraphAttrIndex];
                const auto txtStyle = createTextStyle(textAttrs[styleIndex]);
                if (newLine >> 7 == 0 && newLine == '\n')
                {
                  if (seperateLines || pa.type.lineType != TLT_Plain)
                  {
                    const char* lastStyleEnd = end; // remove newline
                    const auto bytes = lastStyleEnd - prevStyleBegin;
                    builder->pushStyle(txtStyle);
                    builder->addText(prevStyleBegin, bytes);
                    std::cout << "Style Text: [" << std::string(prevStyleBegin, bytes) << "], "
                              << (int)txtStyle.getFontSize() << std::endl;
                    builder->pop();
                    prevStyleBegin = end; // skip newline
                    if (this->length >= offset)
                    {
                      skip = true;
                      styleIndex++;
                      if (styleIndex < textAttrs.size())
                      {
                        offset += textAttrs[styleIndex].length;
                      }
                    }

                    if (pa.type.lineType != TLT_Plain)
                    {
                      if (pa.type.firstLine)
                      {
                        // push stack
                        pushIntendation(intendationOffset);
                        currentLevel = pa.type.intendation;
                        intendationOffset = calcWhitespace(currentLevel,
                                                           txtStyle.getFontSize(),
                                                           txtStyle.getFontFamilies(),
                                                           fontCollection);
                      }
                      else if (pa.type.intendation < currentLevel)
                      {
                        while (currentLevel > pa.type.intendation)
                        {
                          intendationOffset = popIntendation();
                          currentLevel--;
                        }
                        // pop until the current level
                      }
                    }
                    else
                    {
                      currentLevel = 0;
                      intendationOffset = 0;
                    }
                    paragraphs.emplace_back(std::string_view{ prevParagraphBegin, begin },
                                            std::move(builder),
                                            intendationOffset,
                                            paragraphCharCount);

                    std::cout << "Paragraph Text: [" << std::string(prevParagraphBegin, begin)
                              << "], " << currentLevel << ", " << intendationOffset << std::endl;
                  }
                  prevParagraphBegin = begin;
                  if (paragraphAttrIndex + 1 < paragraphAttributes.size())
                  {
                    paragraphAttrIndex++;
                  }
                  if (seperateLines || pa.type.lineType != TLT_Plain)
                  {
                    builder = skia::textlayout::ParagraphBuilder::make(
                      createParagraphStyle(paragraphAttributes[paragraphAttrIndex]),
                      fontCollection);
                  }
                  // print wanring
                  paragraphCharCount = 0;
                }
                if (this->length >= offset && !skip)
                {
                  if (this->length > offset)
                  {
                    WARN("style offset do not match utf8 character count");
                  }
                  assert(builder);

                  const char* lastStyleEnd = end;
                  const auto bytes = lastStyleEnd - prevStyleBegin;
                  builder->pushStyle(txtStyle);
                  builder->addText(prevStyleBegin, bytes);
                  std::cout << "Style Text: [" << std::string(prevStyleBegin, bytes) << "], "
                            << (int)textAttrs[styleIndex].size << std::endl;
                  builder->pop();
                  if (styleIndex + 1 < textAttrs.size())
                  {
                    styleIndex++;
                    prevStyleBegin = lastStyleEnd;
                    offset += textAttrs[styleIndex].length;
                  }
                  else
                  {
                    WARN("No more style for text");
                    builder->pushStyle(txtStyle);
                    builder->addText(prevStyleBegin); // until \0
                    builder->pop();
                    return false;
                  }
                }
                return true;
              });

  auto isUtf8 = (reason != Reason::InvalidUTF8);
  if (!isUtf8)
  {
    WARN("Invalid utf8\n");
  }
  assert(builder);

  const auto pa = paragraphAttributes[paragraphAttrIndex];
  if (pa.type.lineType != TLT_Plain)
  {

    const auto txtStyle = createTextStyle(textAttrs[styleIndex]);
    if (pa.type.firstLine)
    {
      // push stack
      pushIntendation(intendationOffset);
      currentLevel = pa.type.intendation;
      intendationOffset = calcWhitespace(currentLevel,
                                         txtStyle.getFontSize(),
                                         txtStyle.getFontFamilies(),
                                         fontCollection);
    }
    else if (pa.type.intendation < currentLevel)
    {
      while (currentLevel > pa.type.intendation)
      {
        intendationOffset = popIntendation();
        currentLevel--;
      }
      // pop until the current level
    }
  }
  else
  {
    currentLevel = 0;
    intendationOffset = 0;
  }
  paragraphs.emplace_back(std::string_view{ prevParagraphBegin },
                          std::move(builder),
                          intendationOffset,
                          paragraphCharCount);

  return paragraphs;
}
} // namespace VGG
