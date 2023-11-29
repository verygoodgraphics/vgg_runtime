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
#include "Layer/Core/VType.hpp"
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

namespace internal
{

template<class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace internal

class TextParagraphCache
  : public ParagraphListener
  , public VNode
{
public:
  using TextParagraphCacheDirtyFlags = uint8_t;
  enum ETextParagraphCacheFlagsBits : TextParagraphCacheDirtyFlags
  {
    D_REBUILD = 1,
    D_LAYOUT = 2,
    D_ALL = D_REBUILD | D_LAYOUT,
  };

  std::vector<TextParagraph> paragraph;
  std::vector<ParagraphInfo> paragraphCache;

  void setFontCollection(sk_sp<VGGFontCollection> fontCollection)
  {
    this->m_fontCollection = std::move(fontCollection);
  }

private:
  bool m_newParagraph{ true };

  ParagraphAttr                m_paraAttr;
  TextParagraphCacheDirtyFlags m_dirtyFlags{ D_ALL };
  sk_sp<VGGFontCollection>     m_fontCollection;

  enum EState
  {
    EMPTY,
    INIT,
    PARSE,
    BUILT,
    LAYOUT,
  };
  uint8_t m_state{ EMPTY };

  std::string m_utf8Text;

  ETextVerticalAlignment m_verticalAlign;
  TextLayoutMode         m_textLayoutMode;

  Bound                      m_textBound;
  std::vector<TextStyleAttr> m_textStyle;
  std::vector<ParagraphAttr> m_lineStyle;

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
    Bound newBound;
    std::visit(
      internal::Overloaded{ [&](const TextLayoutAutoHeight& l) { newBound = layout(l); },
                            [&](const TextLayoutAutoWidth& l) { newBound = layout(l); },
                            [&](const TextLayoutFixed& l) { newBound = layout(l); } },
      m_textLayoutMode);
    return newBound;
  }

public:
  TextParagraphCache()
  {
    auto mgr = sk_ref_sp(FontManager::instance().defaultFontManager());
    m_fontCollection = sk_make_sp<VGGFontCollection>(std::move(mgr));
  }
  TextParagraphCache(const TextParagraphCache&) = default;
  TextParagraphCache& operator=(const TextParagraphCache&) = default;

  TextParagraphCache(TextParagraphCache&&) noexcept = default;
  TextParagraphCache& operator=(TextParagraphCache&&) noexcept = default;

  TextParagraphCache(sk_sp<VGGFontCollection> fontCollection)
    : m_fontCollection(std::move(fontCollection))
  {
  }

private:
  bool empty() const
  {
    return paragraph.empty();
  }

  bool ensureBuild(TextLayoutMode mode)
  {
    bool ok = false;
    if (m_state <= EMPTY)
      return ok;
    if (m_state < PARSE)
    {
      ParagraphParser p;
      p.parse(*this, m_utf8Text, m_textStyle, m_lineStyle, &mode);
      m_state = PARSE;
    }
    if (!paragraph.empty() && m_state < BUILT)
    {
      paragraphCache.clear();
      paragraphCache.reserve(paragraph.size());
      for (auto& d : paragraph)
      {
        assert(d.builder);
        paragraphCache.push_back(std::move(d.build()));
      }
      m_state = BUILT;
    }
    if (m_state >= BUILT)
    {
      ok = true;
    }
    return ok;
  }

  Bound internalLayout(const Bound& bound, ETextLayoutMode mode)
  {
    Bound      newBound = bound;
    const auto layoutWidth = bound.width();
    float      newWidth = bound.width();
    float      newHeight = bound.height();
    for (int i = 0; i < paragraphCache.size(); i++)
    {
      // auto paragraph = d.builder->Build();
      const auto&   d = paragraph[i];
      const auto&   paragraph = paragraphCache[i].paragraph;
      SkFontMetrics metrics;
      d.builder->getParagraphStyle().getTextStyle().getFontMetrics(&metrics);
      const auto curX = metrics.fAvgCharWidth * d.level;
      paragraphCache[i].offsetX = curX;
      if (mode == ETextLayoutMode::TL_WidthAuto)
      {
        constexpr float MAX_WIDTH = 100000;
        paragraph->layout(MAX_WIDTH);
      }
      else
      {
        paragraph->layout(layoutWidth - curX);
      }
      newWidth = std::max((float)paragraph->getLongestLine(), newWidth);
      auto lastLine = paragraph->lineNumber();
      if (lastLine < 1)
        continue;
      assert(!d.utf8TextView.text.empty());
      auto c = d.utf8TextView.text.back();
      if (c == '\n' && i < paragraphCache.size() - 1)
      {
        // It not a neat design because list item must be rendered seperately.
        // The line height of newline at each end of paragraph except for the last need to be dealt
        // specially
        LineMetrics lineMetric;
        paragraph->getLineMetricsAt(lastLine - 1, &lineMetric);
        newHeight += paragraph->getHeight() - lineMetric.fHeight;
      }
      else
      {
        newHeight += paragraph->getHeight();
      }
    }
    if (mode == ETextLayoutMode::TL_HeightAuto)
    {
      newBound.setHeight(newHeight);
    }
    if (mode == ETextLayoutMode::TL_WidthAuto)
    {
      newBound.setHeight(newHeight);
      newBound.setWidth(newWidth);
    }
    return newBound;
  }

public:
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

  void setTextLayoutMode(TextLayoutMode mode)
  {
    m_textLayoutMode = mode;
    // if (ensureBuild(mode))
    // {
    //   ETextHorizontalAlignment align;
    //   std::visit(
    //     internal::Overloaded{ [&](const TextLayoutAutoHeight& l) {},
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

  Bound layout(TextLayoutFixed mode)
  {
    if (m_state >= LAYOUT)
      return m_textBound;
    ensureBuild(mode);
    m_textBound = internalLayout(mode.textBound, TL_Fixed);
    m_state = LAYOUT;
    return m_textBound;
  }

  Bound layout(TextLayoutAutoWidth mode)
  {
    if (m_state >= LAYOUT)
      return m_textBound;
    ensureBuild(mode);
    m_textBound = internalLayout(Bound(), TL_WidthAuto);
    m_state = LAYOUT;
    return m_textBound;
  }

  Bound layout(TextLayoutAutoHeight mode)
  {
    if (m_state >= LAYOUT)
      return m_textBound;
    ensureBuild(mode);
    Bound b;
    b.setWidth(mode.width);
    m_textBound = internalLayout(b, TL_HeightAuto);
    m_state = LAYOUT;
    return m_textBound;
  }
};

} // namespace VGG::layer
