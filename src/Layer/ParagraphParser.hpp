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
#pragma once
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

#include "Layer/Core/TreeNode.hpp"

namespace VGG::layer
{

using namespace skia::textlayout;

struct TextView
{
  std::string_view text;
  size_t           count;
  TextView() = default;
  TextView(const std::string_view& text, size_t count)
    : text(text)
    , count(count)
  {
  }
};

struct ParagraphAttr
{
  TextLineAttr             type;
  ETextHorizontalAlignment horiAlign{ HA_LEFT };
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

public:
  ParagraphListener() = default;
  ParagraphListener(const ParagraphListener&) = default;
  ParagraphListener& operator=(const ParagraphListener&) = default;

  ParagraphListener(ParagraphListener&&) noexcept = default;
  ParagraphListener& operator=(ParagraphListener&&) noexcept = default;

protected:
  virtual void onBegin() = 0;
  virtual void onEnd() = 0;
  virtual void onParagraphBegin(
    int                  paraIndex,
    int                  order,
    const ParagraphAttr& paragraAttr,
    void*                userData) = 0;
  virtual void onParagraphEnd(int paraIndex, const TextView& textView, void* userData) = 0;
  virtual void onTextStyle(
    int                  paraIndex,
    int                  styleIndex,
    const TextView&      textView,
    const TextStyleAttr& textAttr,
    void*                userData) = 0;
};

class ParagraphParser
{
  int          m_length{ 0 };
  unsigned int m_styleIndex{ 0 };
  unsigned int m_paragraphAttrIndex{ 0 };
  const char*  m_prevStyleBegin{ nullptr };
  const char*  m_prevParagraphBegin{ nullptr };
  int          m_offset{ 0 };
  bool         m_seperateLines{ false };
  struct LevelOrderState
  {
    std::unordered_map<int, int> level2Order;
    void                         reset()
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
    m_styleIndex = 0;
    m_paragraphAttrIndex = 0;
    m_offset = firstOffset;
    m_length = 0;
    m_prevStyleBegin = text.c_str();
    m_prevParagraphBegin = text.c_str();
    m_orderState.reset();
  }

public:
  ParagraphParser(bool seperateLines = false)
    : m_seperateLines(seperateLines)
  {
  }

  void parse(
    ParagraphListener&                listener,
    const std::string&                text,
    const std::vector<TextStyleAttr>& textAttrs,
    const std::vector<ParagraphAttr>& paragraphAttributes,
    void*                             userData = nullptr);
};
} // namespace VGG::layer
