#pragma once
#include "Core/Attrs.h"
#include "Core/VGGType.h"
#include "SkiaBackend/SkiaImpl.h"
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

#include "Core/Node.hpp"

namespace VGG
{

using namespace skia::textlayout;
class TextParagraph : public Node
{
public:
  std::string_view text;
  std::unique_ptr<ParagraphBuilder> builder;
  int characters{ 0 };
  int level{ 0 };

  TextParagraph(std::string_view view,
                std::unique_ptr<ParagraphBuilder> builder,
                int level,
                int charCount)
    : Node("")
    , text(view)
    , builder(std::move(builder))
    , level(level)
    , characters(charCount)
  {
  }
};

struct ParagraphAttr
{
  TextLineAttr type;
  ETextHorizontalAlignment horiAlign;
  int maxLines{ 1000 };
  std::u16string ellipsis{ u"..." };
};
class ParagraphParser
{
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

  int calcWhitespace(int count,
                     int fontSize,
                     const std::vector<SkString>& fontFamilies,
                     sk_sp<FontCollection> fontCollection);

  std::stack<int> m_intendationStack;
  int m_stackTop{ 0 };
  void pushIntendation(int intend)
  {
    // m_stackTop += intend;
    m_intendationStack.push(intend);
  }

  int topIntendation() const
  {
    return m_intendationStack.top();
  }

  int popIntendation()
  {
    if (!m_intendationStack.empty())
    {
      const auto r = m_intendationStack.top();
      m_intendationStack.pop();
      m_stackTop = r;
      return m_stackTop;
    }
    return 0;
  }

public:
  ParagraphParser(bool seperateLines = false)
    : seperateLines(seperateLines)
  {
  }
  std::vector<TextParagraph> parse(const std::string& text,
                                   const std::vector<TextAttr>& textAttrs,
                                   const std::vector<ParagraphAttr>& paragraphAttributes,
                                   sk_sp<FontCollection> fontCollection);

  std::shared_ptr<TextParagraph> parseTree(const std::string& text,
                                           const std::vector<TextAttr>& textAttrs,
                                           const std::vector<ParagraphAttr>& paragraphAttributes,
                                           sk_sp<FontCollection> fontCollection);
};
} // namespace VGG
