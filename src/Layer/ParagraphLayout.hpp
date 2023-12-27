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
  Bound textBound;
  TextLayoutFixed() = default;
  TextLayoutFixed(const Bound& b)
    : textBound(b)
  {
  }
};

using TextLayoutMode = std::variant<TextLayoutFixed, TextLayoutAutoWidth, TextLayoutAutoHeight>;

class RichTextBlock;
#ifdef USE_SHARED_PTR
using RichTextBlockPtr = std::shared_ptr<RichTextBlock>;
#else
using RichTextBlockPtr = VGG::layer::Ref<RichTextBlock>;
#endif

template<typename... Args>
inline RichTextBlockPtr makeRichTextBlockPtr(Args&&... args)
{
#ifdef USE_SHARED_PTR
  auto p = std::make_shared<RichTextBlock>(nullptr);
  return p;
#else
  return RichTextBlockPtr(V_NEW<RichTextBlock>(std::forward<Args>(args)...));
#endif
};

class RichTextBlock final
  : public ParagraphListener
  , public VNode
{
public:
  std::vector<TextParagraph> paragraph;
  std::vector<ParagraphInfo> paragraphCache;

private:
  bool                     m_newParagraph{ true };
  ParagraphAttr            m_paraAttr;
  sk_sp<VGGFontCollection> m_fontCollection;

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
  TextLayoutMode         m_textLayoutMode;

  Bound                      m_textBound;
  std::vector<TextStyleAttr> m_textStyle;
  std::vector<ParagraphAttr> m_lineStyle;

  int m_paragraphHeight{ 0 };

  bool                    ensureBuild(TextLayoutMode mode);
  std::pair<Bound, float> internalLayout(const Bound& bound, ETextLayoutMode mode);

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

  Bound onRevalidate() override
  {
    std::visit(
      Overloaded{
        [&](const TextLayoutAutoHeight& l)
        { std::tie(m_textBound, m_paragraphHeight) = layout(l); },
        [&](const TextLayoutAutoWidth& l) { std::tie(m_textBound, m_paragraphHeight) = layout(l); },
        [&](const TextLayoutFixed& l) { std::tie(m_textBound, m_paragraphHeight) = layout(l); } },
      m_textLayoutMode);
    return m_textBound;
  }

public:
  RichTextBlock(VRefCnt* cnt)
    : VNode(cnt)
  {
    auto mgr = sk_ref_sp(FontManager::instance().defaultFontManager());
    m_fontCollection = sk_make_sp<VGGFontCollection>(std::move(mgr));
  }
  RichTextBlock(const RichTextBlock&) = delete;
  RichTextBlock& operator=(const RichTextBlock&) = delete;

  RichTextBlock(RichTextBlock&&) noexcept = delete;
  RichTextBlock& operator=(RichTextBlock&&) noexcept = delete;

  RichTextBlock(VRefCnt* cnt, sk_sp<VGGFontCollection> fontCollection)
    : VNode(cnt)
    , m_fontCollection(std::move(fontCollection))
  {
  }

public:
  bool empty() const
  {
    return m_state == EMPTY;
  }

  void setText(std::string utf8)
  {
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
    m_textStyle = std::move(styles);
    m_state = INIT;
    invalidate();
  }

  const std::vector<TextStyleAttr>& textStyles() const
  {
    return m_textStyle;
  }

  void setLineStyle(std::vector<ParagraphAttr> styles)
  {
    m_lineStyle = std::move(styles);
    m_state = INIT;
    invalidate();
  }

  const std::vector<ParagraphAttr>& lineStyle() const
  {
    return m_lineStyle;
  }

  void setFontCollection(sk_sp<VGGFontCollection> fontCollection)
  {
    if (this->m_fontCollection == fontCollection)
      return;
    m_fontCollection = std::move(fontCollection);
    m_state = INIT;
    revalidate();
  }

  void setTextLayoutMode(TextLayoutMode mode)
  {
    m_textLayoutMode = mode;
    // if (ensureBuild(mode))
    // {
    //   ETextHorizontalAlignment align;
    //   std::visit(
    //     Overloaded{ [&](const TextLayoutAutoHeight& l) {},
    //                           [&](const TextLayoutAutoWidth& l) { align = HA_Left; },
    //                           [&](const TextLayoutFixed& l) {} },
    //     mode);
    //   for (auto& d : paragraphCache)
    //   {
    //     d.paragraph->updateTextAlign(toSkTextAlign(align));
    //   }
    // }
    invalidate();
  }

  float textHeight() const
  {
    return m_paragraphHeight;
  }

  TextLayoutMode textLayoutMode() const
  {
    return m_textLayoutMode;
  }

  void setVerticalAlignment(ETextVerticalAlignment align)
  {
    if (m_verticalAlign == align)
      return;
    m_verticalAlign = align;
    invalidate();
  }

  ETextVerticalAlignment verticalAlignment() const
  {
    return m_verticalAlign;
  }

  std::pair<Bound, float> layout(TextLayoutFixed mode)
  {
    if (m_state >= LAYOUT)
      return { m_textBound, m_paragraphHeight };
    ensureBuild(mode);
    std::tie(m_textBound, m_paragraphHeight) = internalLayout(mode.textBound, TL_Fixed);
    m_state = LAYOUT;
    return { m_textBound, m_paragraphHeight };
  }

  std::pair<Bound, float> layout(TextLayoutAutoWidth mode)
  {
    if (m_state >= LAYOUT)
      return { m_textBound, m_paragraphHeight };
    ensureBuild(mode);
    std::tie(m_textBound, m_paragraphHeight) = internalLayout(Bound(), TL_WidthAuto);
    m_state = LAYOUT;
    return { m_textBound, m_paragraphHeight };
  }

  std::pair<Bound, float> layout(TextLayoutAutoHeight mode)
  {
    if (m_state >= LAYOUT)
      return { m_textBound, m_paragraphHeight };
    ensureBuild(mode);
    Bound b;
    b.setWidth(mode.width);
    std::tie(m_textBound, m_paragraphHeight) = internalLayout(b, TL_HeightAuto);
    m_state = LAYOUT;
    return { m_textBound, m_paragraphHeight };
  }
};

} // namespace VGG::layer
