#include "Core/Attrs.h"
#include "Core/FontManager.h"
#include "Core/TextNode.h"
#include "Core/VGGType.h"
#include "SkiaBackend/SkiaImpl.h"
#include <core/SkColor.h>
#include <core/SkFontStyle.h>
#include <core/SkScalar.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

namespace VGG
{

using namespace skia::textlayout;

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

struct TextParagraph
{
  std::string_view text;
  std::unique_ptr<ParagraphBuilder> builder;
  int characters{ 0 };
  int level{ 0 };
  TextParagraph(std::string_view view,
                std::unique_ptr<ParagraphBuilder> builder,
                int level,
                int charCount)
    : text(view)
    , builder(std::move(builder))
    , level(level)
    , characters(charCount)
  {
  }
};

class TextParagraphBuilder
{
public:
  struct ParagraphAttr
  {
    TextLineAttr type;
    ETextHorizontalAlignment horiAlign;
    int maxLines{ 1000 };
    std::u16string ellipsis{ u"..." };
  };

private:
  int length{ 0 };
  int styleIndex{ 0 };
  int paragraphAttrIndex{ 0 };
  const char* prevStyleBegin{ nullptr };
  const char* prevParagraphBegin{ nullptr };
  int offset{ 0 };

  bool seperateLines{ false };
  void reset(const std::string& text, int firstOffset)
  {
    styleIndex = 0;
    paragraphAttrIndex = 0;
    offset = firstOffset;
    length = 0;
    prevStyleBegin = text.c_str();
    prevParagraphBegin = text.c_str();
  }

  static skia::textlayout::ParagraphStyle createParagraphStyle(const ParagraphAttr& attr)
  {
    ParagraphStyle style;
    style.setEllipsis(attr.ellipsis);
    style.setTextAlign(TextAlign::kLeft);
    style.setMaxLines(attr.maxLines);
    return style;
  }

  static skia::textlayout::TextStyle createTextStyle(const TextAttr& attr)
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
  int fromTab(int count,
              int fontSize,
              const std::vector<SkString>& fontFamilies,
              sk_sp<FontCollection> fontCollection)
  {
    // cassert(count < 40);
    ParagraphStyle style;
    style.setEllipsis(u"...");
    style.setTextAlign(TextAlign::kLeft);
    style.setMaxLines(100);
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
    p->layout(100);
    return 30;
  }

public:
  TextParagraphBuilder(bool seperateLines = false)
    : seperateLines(seperateLines)
  {
  }
  std::vector<TextParagraph> operator()(const std::string& text,
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
    int intendation = 0;
    int oldStyleIndex = styleIndex;

    auto reason =
      runOnUtf8(text.c_str(),
                text.size(),
                [&, this](const char* begin, const char* end, int charCount)
                {
                  this->length += charCount;
                  paragraphCharCount += charCount;
                  unsigned char newLine = *begin;
                  bool skip = false;
                  auto pa = paragraphAttributes[paragraphAttrIndex];
                  if (newLine >> 7 == 0 && newLine == '\n')
                  {
                    if (seperateLines || pa.type.lineType != TLT_Plain)
                    {
                      const char* lastStyleEnd = end; // remove newline
                      const auto bytes = lastStyleEnd - prevStyleBegin;
                      auto txtStyle = createTextStyle(textAttrs[styleIndex]);
                      builder->pushStyle(txtStyle);
                      builder->addText(prevStyleBegin, bytes);
                      std::cout << "Style Text: [" << std::string(prevStyleBegin, bytes) << "], "
                                << (int)textAttrs[styleIndex].size << std::endl;
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
                      if (pa.type.firstLine)
                      {
                        intendation = fromTab(pa.type.intendation,
                                              textAttrs[styleIndex].size,
                                              txtStyle.getFontFamilies(),
                                              fontCollection);
                        oldStyleIndex = styleIndex;
                      }
                      else
                      {
                        auto txtStyle = createTextStyle(textAttrs[oldStyleIndex]);
                        intendation = fromTab(pa.type.intendation,
                                              txtStyle.getFontSize(),
                                              txtStyle.getFontFamilies(),
                                              fontCollection);
                      }
                      std::cout << "intendation: " << intendation << std::endl;
                      paragraphs.emplace_back(std::string_view{ prevParagraphBegin, begin },
                                              std::move(builder),
                                              intendation,
                                              paragraphCharCount);
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
                    builder->pushStyle(createTextStyle(textAttrs[styleIndex]));
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
                      // builder->pushStyle(createStyle(textAttrs[styleIndex]));
                      // builder->addText(prevStyleBegin); // until \0
                      // builder->pop();
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
    paragraphs.emplace_back(std::string_view{ prevParagraphBegin },
                            std::move(builder),
                            intendation,
                            paragraphCharCount);

    return paragraphs;
  }
};

struct CursorState
{
  SkScalar cursorX = 0.f;
  SkScalar cursorY = 0.f;
  SkScalar layoutWidth = 500.f;

  void advanceX(SkScalar a)
  {
    cursorX += a;
    layoutWidth -= a;
  }
  void advanceY(SkScalar a)
  {
    cursorY += a;
  }

  void reset(SkScalar layout)
  {
    cursorY = 0;
    cursorX = 0;
    layoutWidth = layout;
  }
};

class DebugCanvas
{
public:
  DebugCanvas(SkCanvas* canvas)
    : canvas(canvas)
  {
    canvas->clear(SK_ColorWHITE);
  }

  void drawRects(SkColor color, std::vector<TextBox>& result, bool fill = false)
  {

    SkPaint paint;
    if (!fill)
    {
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setAntiAlias(true);
      paint.setStrokeWidth(1);
    }
    paint.setColor(color);
    for (auto& r : result)
    {
      canvas->drawRect(r.rect, paint);
    }
  }

  void drawLine(SkColor color, SkRect rect, bool vertical = true)
  {

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1);
    paint.setColor(color);
    if (vertical)
    {
      canvas->drawLine(rect.fLeft, rect.fTop, rect.fLeft, rect.fBottom, paint);
    }
    else
    {
      canvas->drawLine(rect.fLeft, rect.fTop, rect.fRight, rect.fTop, paint);
    }
  }

  void drawLines(SkColor color, std::vector<TextBox>& result)
  {

    for (auto& r : result)
    {
      drawLine(color, r.rect);
    }
  }

  SkCanvas* get()
  {
    return canvas;
  }

private:
  SkCanvas* canvas;
  const char* name;
};

inline void drawParagraphDebugInfo(DebugCanvas& canvas,
                                   const TextParagraph& textParagraph,
                                   Paragraph* p,
                                   CursorState& state,
                                   int index)
{
  static SkColor colorTable[9] = {
    SK_ColorBLUE,   SK_ColorGREEN,  SK_ColorRED,  SK_ColorCYAN,   SK_ColorMAGENTA,
    SK_ColorYELLOW, SK_ColorDKGRAY, SK_ColorGRAY, SK_ColorLTGRAY,
  };
  canvas.get()->save();
  canvas.get()->translate(state.cursorX, state.cursorY);
  auto rects =
    p->getRectsForRange(0, textParagraph.characters, RectHeightStyle::kMax, RectWidthStyle::kMax);
  auto h = p->getHeight();
  auto mw = p->getMaxWidth();
  auto iw = p->getMaxIntrinsicWidth();
  SkPaint pen;
  SkColor color = colorTable[index % 9];
  pen.setColor(SkColorSetA(color, 0x88));
  canvas.get()->drawRect(SkRect{ 0, 0, mw, h }, pen);
  // pen.setColor(SK_ColorBLUE);
  // canvas.get()->drawRect(SkRect{0, 0, iw, h}, pen);
  canvas.drawRects(color, rects);
  canvas.get()->restore();
}

class TextParagraphCache
{
  std::vector<TextParagraph> paragraph;

  std::vector<std::unique_ptr<skia::textlayout::Paragraph>> paragraphCache;
  bool m_dirty{ true };
  void clear()
  {
    m_dirty = false;
  }

public:
  TextParagraphCache() = default;
  TextParagraphCache(std::vector<TextParagraph> paragraph)
    : paragraph(std::move(paragraph))
  {
  }

  bool isDirty() const
  {
    return m_dirty;
  }

  void cache()
  {
    if (!isDirty())
      return;
    paragraphCache.clear();
    paragraphCache.reserve(paragraph.size());
    for (auto& d : paragraph)
    {
      paragraphCache.push_back(std::move(d.builder->Build()));
    }
    clear();
  }

  void drawParagraph(SkCanvas* canvas, const Bound2& bound)
  {
    if (paragraph.empty())
      return;
    cache();
    CursorState cursor;
    const auto layoutWidth = bound.width();
    cursor.reset(layoutWidth);

    DebugCanvas debugCanvas(canvas);
    for (int i = 0; i < paragraphCache.size(); i++)
    {
      auto& p = paragraphCache[i];
      p->layout(cursor.layoutWidth);
      p->paint(canvas, cursor.cursorX, cursor.cursorY);
      auto lastLine = p->lineNumber();
      if (lastLine < 1)
        continue;
      LineMetrics lineMetric;
      p->getLineMetricsAt(lastLine - 1, &lineMetric);
      const auto height = p->getHeight() - lineMetric.fHeight;
      const auto width = paragraph[i].level;
      if (Scene::isEnableDrawDebugBound())
      {
        drawParagraphDebugInfo(debugCanvas, paragraph[i], p.get(), cursor, i);
      }
      cursor.advanceY(height);
      cursor.cursorX = width;
    }
  }
};

class TextNode__pImpl
{
  VGG_DECL_API(TextNode)
public:
  TextNode__pImpl(TextNode* api)
    : q_ptr(api)
  {
  }
  std::string text;
  std::vector<TextStyleStub> styles;
  TextParagraphCache m_paragraphCache;
  CursorState m_cursorState;
  ETextLayoutMode mode;
  ETextVerticalAlignment m_vertAlign{ ETextVerticalAlignment::VA_Top };
};
} // namespace VGG
