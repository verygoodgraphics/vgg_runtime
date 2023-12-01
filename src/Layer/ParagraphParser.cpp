/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ParagraphParser.hpp"
#include "Utility/Log.hpp"
#include "Layer/Core/VType.hpp"

#include <iterator>
#include <list>

namespace VGG::layer
{

enum class EReason
{
  FINISH,
  TERMINATE,
  INVALID_UTF8,
};
template<typename F>
EReason runOnUtf8(const char* utf8, size_t bytes, F&& f)
{
  const char* cur = utf8;
  const char* nextChar = nullptr;
  int         charCount = 0;
  int         bytesCount = 0;
  auto        nextUtf8Char = [](unsigned char* pBegin, int& charCount) -> const char*
  {
    if (*pBegin >> 7 == 0)
    {
      charCount = 1;
      ++pBegin;
    }
    else if (*pBegin >> 5 == 6 && pBegin[1] >> 6 == 2)
    {
      pBegin += 2;
      charCount = 1;
    }
    else if (*pBegin >> 4 == 0x0E && pBegin[1] >> 6 == 2 && pBegin[2] >> 6 == 2)
    {
      pBegin += 3;
      charCount = 1;
    }
    else if (
      *pBegin >> 3 == 0x1E && pBegin[1] >> 6 == 2 && pBegin[2] >> 6 == 2 && pBegin[3] >> 6 == 2)
    {
      pBegin += 4;
      charCount = 2;
    }
    else
    {
      assert(false && "invalid utf8");
      return nullptr;
    }
    return (const char*)pBegin;
  };
  while ((nextChar = nextUtf8Char((unsigned char*)cur, charCount)))
  {
    bytesCount += (nextChar - cur);
    if (bytesCount > bytes)
    {
      return EReason::FINISH;
    }
    if (!f(cur, nextChar, charCount))
      return EReason::TERMINATE;
    cur = nextChar;
  }
  return EReason::INVALID_UTF8;
}
void ParagraphParser::parse(
  ParagraphListener&                listener,
  const std::string&                text,
  const std::vector<TextStyleAttr>& textAttrs,
  const std::vector<ParagraphAttr>& paragraphAttributes,
  void*                             userData)
{
  if (text.empty() || paragraphAttributes.empty() || textAttrs.empty())
    return;
  reset(text, textAttrs[0].length);
  listener.onBegin();
  size_t paragraphCharCount = 0;
  size_t styleTextCharCount = 0;
  int    currentOrder = 0;
  listener.onParagraphBegin(
    m_paragraphAttrIndex,
    currentOrder,
    paragraphAttributes[m_paragraphAttrIndex],
    userData);
  auto reason = runOnUtf8(
    text.c_str(),
    text.size(),
    [&, this](const char* begin, const char* end, int charCount)
    {
      this->m_length += charCount;
      paragraphCharCount += charCount;
      styleTextCharCount += charCount;
      unsigned char newLine = *begin;
      bool          skip = false;
      const auto    pa = paragraphAttributes[m_paragraphAttrIndex];
      if (newLine >> 7 == 0 && newLine == '\n')
      {
        const auto breakLine =
          m_seperateLines || pa.type.lineType != TLT_Plain || m_styleIndex + 1 < textAttrs.size()
            ? textAttrs[m_styleIndex + 1].horzAlignment != textAttrs[m_styleIndex].horzAlignment
            : false;
        if (breakLine)
        {
          const char* lastStyleEnd = end;
          const auto  bytes = lastStyleEnd - m_prevStyleBegin;
          listener.onTextStyle(
            m_paragraphAttrIndex,
            m_styleIndex,
            { std::string_view(m_prevStyleBegin, bytes), styleTextCharCount },
            textAttrs[m_styleIndex],
            userData);
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
              paragraphCharCount },
            userData);
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
          listener.onParagraphBegin(
            m_paragraphAttrIndex,
            currentOrder,
            paragraphAttributes[m_paragraphAttrIndex],
            userData);
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
        const auto  bytes = lastStyleEnd - m_prevStyleBegin;
        listener.onTextStyle(
          m_paragraphAttrIndex,
          m_styleIndex,
          { std::string_view(m_prevStyleBegin, bytes), styleTextCharCount },
          textAttrs[m_styleIndex],
          userData);
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

  auto isUtf8 = (reason != EReason::INVALID_UTF8);
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
      textAttrs[m_styleIndex],
      userData);
  }
  styleTextCharCount = 0;
  assert(m_paragraphAttrIndex < paragraphAttributes.size());
  listener.onParagraphEnd(
    m_paragraphAttrIndex,
    TextView{ std::string_view{ m_prevParagraphBegin }, paragraphCharCount },
    userData);
  paragraphCharCount = 0;
  listener.onEnd();
}

} // namespace VGG::layer
