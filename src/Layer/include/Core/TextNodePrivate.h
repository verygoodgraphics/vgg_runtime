#include "Core/Attrs.h"
#include "Core/TextNode.h"
#include "Core/VGGType.h"
#include <modules/skparagraph/include/DartTypes.h>
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

inline skia::textlayout::TextStyle createStyle(const TextAttr& attr)
{
  skia::textlayout::TextStyle style;
  style.setColor(SK_ColorBLACK);
  style.setFontFamilies({ SkString("Roboto"), SkString("Noto Color Emoji") });
  style.setFontSize(attr.size);
  style.setLetterSpacing(attr.letterSpacing);
  // ftxtStyle.setWordSpacing(attr.);
  // txtStyle.setHeight(100);
  return style;
}

struct ParagraphSet
{
  struct ParagraphAttr
  {
    ParagraphStyle defaultParagraphStyle;
    TextLineAttr type;
  };

  struct Paragraph
  {
    std::string_view text;
    std::unique_ptr<ParagraphBuilder> builder;
    int level{ 0 };
    Paragraph(std::string_view view, std::unique_ptr<ParagraphBuilder> builder, int level)
      : text(view)
      , builder(std::move(builder))
      , level(level)
    {
    }
  };

  std::string Text;
  int Length{ 0 };
  std::vector<Paragraph> Paragraphs;
  bool IsUtf8{ true };

  ParagraphSet(std::string text,
               const std::vector<TextAttr>& textAttrs,
               const std::vector<ParagraphAttr>& paragraphAttributes,
               sk_sp<FontCollection> fontCollection)
    : Text(std::move(text))
  {
    assert(!textAttrs.empty());
    assert(!paragraphAttributes.empty());
    assert(!Text.empty());
    int styleIndex = 0;
    int paragraphAttrIndex = 0;
    const char* prevStyleBegin = Text.data();
    const char* prevParagraphBegin = Text.data();

    int offset = textAttrs[styleIndex].length;
    auto builder = skia::textlayout::ParagraphBuilder::make(
      paragraphAttributes[paragraphAttrIndex].defaultParagraphStyle,
      fontCollection);

    auto reason = runOnUtf8(Text.c_str(),
                            Text.size(),
                            [&, this](const char* cur, const char* next, int count)
                            {
                              this->Length += count;
                              unsigned char newLine = *cur;
                              if (newLine >> 7 == 0 && newLine == '\n')
                              {
                                this->Paragraphs.push_back(Paragraph{
                                  std::string_view{ prevParagraphBegin, cur + 1 },
                                  std::move(builder),
                                  paragraphAttributes[paragraphAttrIndex].type.intendation });

                                prevParagraphBegin = cur;
                                if (paragraphAttrIndex + 1 < paragraphAttributes.size())
                                {
                                  paragraphAttrIndex++;
                                  builder = skia::textlayout::ParagraphBuilder::make(
                                    paragraphAttributes[paragraphAttrIndex].defaultParagraphStyle,
                                    fontCollection);
                                }
                                else
                                {
                                  // default linetype
                                  WARN("there is no matched line type for this paragrah");
                                  builder = skia::textlayout::ParagraphBuilder::make(
                                    paragraphAttributes[paragraphAttrIndex].defaultParagraphStyle,
                                    fontCollection);
                                  // print wanring
                                }
                              }
                              if (this->Length >= offset)
                              {
                                if (this->Length > offset)
                                {
                                  WARN("style offset do not match utf8 character count");
                                }
                                assert(builder);

                                const char* lastStyleEnd = cur;
                                const auto bytes = lastStyleEnd - prevStyleBegin;
                                builder->pushStyle(createStyle(textAttrs[styleIndex]));
                                builder->addText(prevStyleBegin, bytes);
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
                                  builder->pushStyle(createStyle(textAttrs[styleIndex]));
                                  builder->addText(cur); // until \0
                                  builder->pop();
                                  return false;
                                }
                              }
                              return true;
                            });

    IsUtf8 = (reason != Reason::InvalidUTF8);
    if (!IsUtf8)
    {
      WARN("Invalid utf8\n");
    }
    assert(builder);
    this->Paragraphs.push_back(
      ParagraphSet::Paragraph{ std::string_view{ prevStyleBegin },
                               std::move(builder),
                               paragraphAttributes[paragraphAttrIndex].type.intendation });

    // process last style
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

  std::unique_ptr<ParagraphSet> m_paragraphSet;
  std::vector<std::unique_ptr<Paragraph>> m_paragraphs;
  ETextLayoutMode mode;
  ETextVerticalAlignment m_vertAlign{ ETextVerticalAlignment::VA_Top };
  void drawText(SkCanvas* canvas);
  void drawParagraph(SkCanvas* canvas)
  {
    assert(m_paragraphSet);
    if (m_paragraphs.empty())
    {
      for (auto& pb : m_paragraphSet->Paragraphs)
      {
        m_paragraphs.push_back(std::move(pb.builder->Build()));
      }
    }
    const auto& b = q_ptr->getBound();
  }
};
} // namespace VGG
