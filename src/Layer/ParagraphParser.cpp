#include "ParagraphParser.hpp"
#include "Utility/Log.hpp"
#include "Layer/Core/VType.hpp"

#include <iterator>
#include <list>

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
void ParagraphParser::parse(ParagraphListener& listener,
                            const std::string& text,
                            const std::vector<TextAttr>& textAttrs,
                            const std::vector<ParagraphAttr>& paragraphAttributes)
{
  assert(!textAttrs.empty());
  assert(!paragraphAttributes.empty());
  assert(!text.empty());

  reset(text, textAttrs[0].length);
  listener.onBegin();
  size_t paragraphCharCount = 0;
  size_t styleTextCharCount = 0;
  int currentOrder = 0;
  listener.onParagraphBegin(m_paragraphAttrIndex,
                            currentOrder,
                            paragraphAttributes[m_paragraphAttrIndex]);
  auto reason = runOnUtf8(
    text.c_str(),
    text.size(),
    [&, this](const char* begin, const char* end, int charCount)
    {
      this->m_length += charCount;
      paragraphCharCount += charCount;
      styleTextCharCount += charCount;
      unsigned char newLine = *begin;
      bool skip = false;
      const auto pa = paragraphAttributes[m_paragraphAttrIndex];
      if (newLine >> 7 == 0 && newLine == '\n')
      {
        const auto breakLine =
          m_seperateLines || pa.type.lineType != TLT_Plain || m_styleIndex + 1 < textAttrs.size()
            ? textAttrs[m_styleIndex + 1].horzAlignment != textAttrs[m_styleIndex].horzAlignment
            : false;
        if (breakLine)
        {
          const char* lastStyleEnd = end;
          const auto bytes = lastStyleEnd - m_prevStyleBegin;
          listener.onTextStyle(m_paragraphAttrIndex,
                               m_styleIndex,
                               { std::string_view(m_prevStyleBegin, bytes), styleTextCharCount },
                               textAttrs[m_styleIndex]);
          styleTextCharCount = 0;
          m_prevStyleBegin = end;
          if (this->m_length >= m_offset)
          {
            skip = true;
            if (m_styleIndex + 1 < textAttrs.size())
            {
              m_styleIndex++;
              m_offset += textAttrs[m_styleIndex].length;
            }
          }

          listener.onParagraphEnd(
            m_paragraphAttrIndex,
            { std::string_view{ m_prevParagraphBegin, size_t(end - m_prevParagraphBegin) },
              paragraphCharCount });
          m_prevParagraphBegin = end;
          if (m_paragraphAttrIndex + 1 < paragraphAttributes.size())
          {
            m_paragraphAttrIndex++;
          }
          currentOrder = 0;
          if (pa.type.lineType != TLT_Plain)
          {
            currentOrder = m_orderState.order(pa.type.level, pa.type.firstLine);
          }
          listener.onParagraphBegin(m_paragraphAttrIndex,
                                    currentOrder,
                                    paragraphAttributes[m_paragraphAttrIndex]);
          // print wanring
          paragraphCharCount = 0;
        }
        // update to new paragraph
      }
      if (!skip && this->m_length >= m_offset)
      {
        if (this->m_length > m_offset)
        {
          WARN("style offset do not match utf8 character count");
        }
        const char* lastStyleEnd = end;
        const auto bytes = lastStyleEnd - m_prevStyleBegin;
        listener.onTextStyle(m_paragraphAttrIndex,
                             m_styleIndex,
                             { std::string_view(m_prevStyleBegin, bytes), styleTextCharCount },
                             textAttrs[m_styleIndex]);
        m_prevStyleBegin = lastStyleEnd;
        styleTextCharCount = 0;
        if (m_styleIndex + 1 < textAttrs.size())
        {
          m_styleIndex++;
          m_offset += textAttrs[m_styleIndex].length; // advanced
        }
      }
      return true;
    });

  auto isUtf8 = (reason != Reason::InvalidUTF8);
  if (!isUtf8)
  {
    WARN("Invalid utf8\n");
  }
  assert(m_styleIndex < textAttrs.size());
  if (m_prevStyleBegin - text.data() < text.size())
  {
    WARN("No more style for text");
    listener.onTextStyle(
      m_paragraphAttrIndex,
      m_styleIndex,
      { std::string_view(m_prevStyleBegin, text.size() - size_t(m_prevStyleBegin - text.data())),
        styleTextCharCount },
      textAttrs[m_styleIndex]);
  }
  styleTextCharCount = 0;
  assert(m_paragraphAttrIndex < paragraphAttributes.size());
  listener.onParagraphEnd(m_paragraphAttrIndex,
                          TextView{ std::string_view{ m_prevParagraphBegin }, paragraphCharCount });
  paragraphCharCount = 0;
  listener.onEnd();
}

} // namespace VGG
