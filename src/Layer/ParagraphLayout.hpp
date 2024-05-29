/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VNode.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Memory/VNew.hpp"
#include "Layer/Memory/VRefCnt.hpp"
#include "Layer/VSkia.hpp"
#include "ParagraphParser.hpp"
#include "DebugCanvas.hpp"

#include "Layer/FontManager.hpp"
#include "VSkFontMgr.hpp"

#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphPainter.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>
#include <tuple>
#include <utility>
#include <variant>

namespace VGG::layer
{

namespace sktxt = skia::textlayout;
struct ParagraphInfo
{
  int                               offsetX{ 0 };
  std::unique_ptr<sktxt::Paragraph> paragraph;
};

class TextParagraph
{

public:
  std::unique_ptr<ParagraphBuilder> builder{ nullptr };
  int                               level{ 0 };
  TextView                          utf8TextView;
  TextParagraph() = default;
  TextParagraph(
    std::string_view                  view,
    std::unique_ptr<ParagraphBuilder> builder,
    int                               level,
    size_t                            charCount)
    : builder(std::move(builder))
    , level(level)
    , utf8TextView({ view, charCount })
  {
  }

  ParagraphInfo build()
  {
    return { level, builder->Build() };
  }
};

void drawParagraphDebugInfo(
  DebugCanvas&         canvas,
  const TextParagraph& textParagraph,
  Paragraph*           p,
  int                  curX,
  int                  curY,
  int                  index);

struct TextLayoutAutoHeight
{
  float width;
  TextLayoutAutoHeight() = default;
  TextLayoutAutoHeight(float width)
    : width(width)
  {
  }
};

struct TextLayoutAutoWidth
{
};

struct TextLayoutFixed
{
  Bounds textBounds;
  TextLayoutFixed() = default;
  TextLayoutFixed(const Bounds& b)
    : textBounds(b)
  {
  }
};

using TextLayoutMode = std::variant<TextLayoutFixed, TextLayoutAutoWidth, TextLayoutAutoHeight>;

class RichTextBlock;
using RichTextBlockPtr = VGG::layer::Ref<RichTextBlock>;

template<typename... Args>
inline RichTextBlockPtr makeRichTextBlockPtr(Args&&... args)
{
  return RichTextBlockPtr(V_NEW<RichTextBlock>(std::forward<Args>(args)...));
};

class RichTextBlock final
  : public ParagraphListener
  , public VNode
{
public:
  std::vector<TextParagraph> paragraph;
  std::vector<ParagraphInfo> paragraphCache;

private:
  bool          m_newParagraph{ true };
  ParagraphAttr m_paraAttr;

  enum EState
  {
    EMPTY,
    INIT,
    PARSE,
    BUILT,
    LAYOUT,
  };
  uint8_t m_state{ EMPTY };

  std::string            m_utf8Text;
  ETextVerticalAlignment m_verticalAlign;
  ETextLayoutMode        m_layoutMode;

  std::vector<TextStyleAttr> m_textStyle;
  std::vector<ParagraphAttr> m_lineStyle;
  sk_sp<FontCollection>      m_fontCollection;

  Bounds m_hintBounds;

  int m_paragraphHeight{ 0 };

  bool                     ensureBuild(TextLayoutMode mode);
  std::pair<Bounds, float> internalLayout(const Bounds& bounds, ETextLayoutMode mode);

protected:
  void onBegin() override;
  void onEnd() override;
  void onParagraphBegin(int paraIndex, int order, const ParagraphAttr& paragraAttr, void* userData)
    override;
  void onParagraphEnd(int paraIndex, const TextView& textView, void* userData) override;
  void onTextStyle(
    int                  paraIndex,
    int                  styleIndex,
    const TextView&      textView,
    const TextStyleAttr& textAttr,
    void*                userData) override;

  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override
  {
    TextLayoutMode mode;
    switch (m_layoutMode)
    {
      case TL_FIXED:
        mode = TextLayoutFixed(m_hintBounds);
        break;
      case TL_AUTOWIDTH:
        mode = TextLayoutAutoWidth();
        break;
      case TL_AUTOHEIGHT:
        mode = TextLayoutAutoHeight(m_hintBounds.width());
        break;
    }
    Bounds newBounds;
    std::visit(
      Overloaded{
        [&](const TextLayoutAutoHeight& l) { std::tie(newBounds, m_paragraphHeight) = layout(l); },
        [&](const TextLayoutAutoWidth& l) { std::tie(newBounds, m_paragraphHeight) = layout(l); },
        [&](const TextLayoutFixed& l) { std::tie(newBounds, m_paragraphHeight) = layout(l); } },
      mode);
    return newBounds;
  }

public:
  RichTextBlock(VRefCnt* cnt, sk_sp<VGGFontCollection> fontCollection)
    : VNode(cnt)
    , m_fontCollection(std::move(fontCollection))
  {
  }
  RichTextBlock(const RichTextBlock&) = delete;
  RichTextBlock& operator=(const RichTextBlock&) = delete;

  RichTextBlock(RichTextBlock&&) noexcept = delete;
  RichTextBlock& operator=(RichTextBlock&&) noexcept = delete;

public:
  bool empty() const
  {
    return m_state == EMPTY;
  }

  void setText(std::string utf8)
  {
    if (m_utf8Text == utf8)
      return;
    m_utf8Text = std::move(utf8);
    m_state = INIT;
    invalidate();
  }

  const std::string& text() const
  {
    return m_utf8Text;
  }

  void setTextStyle(std::vector<TextStyleAttr> styles)
  {
    if (m_textStyle == styles)
      return;
    m_textStyle = std::move(styles);
    m_state = INIT;
    invalidate();
  }

  void setParagraphHintBounds(const Bounds& bounds)
  {
    if (m_hintBounds == bounds)
      return;
    m_hintBounds = bounds;
    invalidate();
  }

  const Bounds& getParagraphHintBounds() const
  {
    return m_hintBounds;
  }

  const std::vector<TextStyleAttr>& textStyles() const
  {
    return m_textStyle;
  }

  void setLineStyle(std::vector<ParagraphAttr> styles)
  {
    if (m_lineStyle == styles)
      return;
    m_lineStyle = std::move(styles);
    m_state = INIT;
    invalidate();
  }

  const std::vector<ParagraphAttr>& lineStyle() const
  {
    return m_lineStyle;
  }

  void setTextLayoutMode(ETextLayoutMode layoutMode)
  {
    if (m_layoutMode == layoutMode)
      return;
    m_layoutMode = layoutMode;
    invalidate();
  }

  const ETextLayoutMode& getTextLayoutMode() const
  {
    return m_layoutMode;
  }

  float firstBaseline() const
  {
    ASSERT(paragraphCache.empty() == false);
    LineMetrics metrics;
    paragraphCache[0].paragraph->getLineMetricsAt(0, &metrics);
    return metrics.fBaseline;
  }

  float textHeight() const
  {
    return m_paragraphHeight;
  }

  void setVerticalAlignment(ETextVerticalAlignment align)
  {
    if (m_verticalAlign == align)
      return;
    m_verticalAlign = align;
    invalidate();
  }

  const ETextVerticalAlignment& getVerticalAlignment() const
  {
    return m_verticalAlign;
  }

  std::pair<Bounds, float> layout(TextLayoutFixed mode)
  {
    if (m_state >= LAYOUT)
      return { m_hintBounds, m_paragraphHeight };
    ensureBuild(mode);
    Bounds newBounds;
    std::tie(newBounds, m_paragraphHeight) = internalLayout(mode.textBounds, TL_FIXED);
    m_state = LAYOUT;
    return { newBounds, m_paragraphHeight };
  }

  std::pair<Bounds, float> layout(TextLayoutAutoWidth mode)
  {
    if (m_state >= LAYOUT)
      return { m_hintBounds, m_paragraphHeight };
    ensureBuild(mode);
    Bounds newBounds;
    std::tie(newBounds, m_paragraphHeight) = internalLayout(Bounds(), TL_AUTOWIDTH);
    m_state = LAYOUT;
    return { newBounds, m_paragraphHeight };
  }

  std::pair<Bounds, float> layout(TextLayoutAutoHeight mode)
  {
    if (m_state >= LAYOUT)
      return { m_hintBounds, m_paragraphHeight };
    ensureBuild(mode);
    Bounds b;
    b.setWidth(mode.width);
    Bounds newBounds;
    std::tie(newBounds, m_paragraphHeight) = internalLayout(b, TL_AUTOHEIGHT);
    m_state = LAYOUT;
    return { newBounds, m_paragraphHeight };
  }
};

} // namespace VGG::layer
