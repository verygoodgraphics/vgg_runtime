#pragma once
#include "Core/Attrs.h"
#include "Core/VType.h"
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

struct TextView
{
  std::string_view Text;
  size_t Count;
  TextView() = default;
  TextView(const std::string_view& text, size_t count)
    : Text(text)
    , Count(count)
  {
  }
};
class TextParagraph
{
public:
  std::unique_ptr<ParagraphBuilder> builder{ nullptr };
  int level{ 0 };
  TextView Utf8TextView;
  TextParagraph() = default;
  TextParagraph(std::string_view view,
                std::unique_ptr<ParagraphBuilder> builder,
                int level,
                size_t charCount)
    : builder(std::move(builder))
    , level(level)
    , Utf8TextView({ view, charCount })
  {
  }
};

struct ParagraphAttr
{
  TextLineAttr type;
  ETextHorizontalAlignment horiAlign;
  ParagraphAttr() = default;
  ParagraphAttr(TextLineAttr type, ETextHorizontalAlignment align)
    : type(type)
    , horiAlign(align)
  {
  }
};

class ParagraphListener
{
  friend class ParagraphParser;

protected:
  virtual void onBegin() = 0;
  virtual void onEnd() = 0;
  virtual void onParagraphBegin(int paraIndex, int order, const ParagraphAttr& paragraAttr) = 0;
  virtual void onParagraphEnd(int paraIndex, const TextView& textView) = 0;
  virtual void onTextStyle(int paraIndex,
                           int styleIndex,
                           const TextView& textView,
                           const TextAttr& textAttr) = 0;
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
  struct LevelOrderState
  {
    std::unordered_map<int, int> level2Order;
    void reset()
    {
      level2Order.clear();
    }
    int order(int currentLevel, int isFirstLine)
    {
      int currentOrder = 0;
      if (!isFirstLine)
      {
        currentOrder = level2Order[currentLevel] + 1;
        level2Order[currentLevel] = currentOrder;
      }
      else
      {
        // reset to 0 for the first line
        level2Order[currentLevel] = currentOrder;
      }
      return currentOrder;
    }
  } m_orderState;
  void reset(const std::string& text, int firstOffset)
  {
    styleIndex = 0;
    paragraphAttrIndex = 0;
    offset = firstOffset;
    length = 0;
    prevStyleBegin = text.c_str();
    prevParagraphBegin = text.c_str();
    m_orderState.reset();
  }

public:
  ParagraphParser(bool seperateLines = false)
    : seperateLines(seperateLines)
  {
  }

  void parse(ParagraphListener& listener,
             const std::string& text,
             const std::vector<TextAttr>& textAttrs,
             const std::vector<ParagraphAttr>& paragraphAttributes);
};
} // namespace VGG
