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
#include "Layer/Core/Attrs.hpp"
#include "Layer/FontManager.hpp"
#include "Layer/Core/TextNode.hpp"
#include "Layer/Core/VType.hpp"
#include "VSkia.hpp"
#include "VSkFontMgr.hpp"
#include "DebugCanvas.hpp"
#include "ParagraphParser.hpp"
#include "ParagraphPainter.hpp"

#include <include/core/SkColor.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkRefCnt.h>
#include <include/core/SkScalar.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphPainter.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

namespace sktxt = skia::textlayout;

namespace
{

#define cauto const auto // NOLINT
void printShapingInfo(sktxt::Paragraph& p)
{
  cauto maxWidth = p.getMaxWidth();
  cauto height = p.getHeight();
}

#undef catuo
} // namespace

namespace VGG::layer
{

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

class TextParagraphCache : public ParagraphListener
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
  int  m_height{ 0 };
  bool m_newParagraph{ true };

  ParagraphAttr                m_paraAttr;
  TextParagraphCacheDirtyFlags m_dirtyFlags{ D_ALL };
  sk_sp<VGGFontCollection>     m_fontCollection;

protected:
  void onBegin() override;
  void onEnd() override;
  void onParagraphBegin(int paraIndex, int order, const ParagraphAttr& paragraAttr) override;
  void onParagraphEnd(int paraIndex, const TextView& textView) override;
  void onTextStyle(
    int                  paraIndex,
    int                  styleIndex,
    const TextView&      textView,
    const TextStyleAttr& textAttr) override;

public:
  TextParagraphCache(Bound bound)
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
  bool empty() const
  {
    return paragraph.empty();
  }

  bool isDirty() const
  {
    return m_dirtyFlags != 0;
  }

  void clear(ETextParagraphCacheFlagsBits bit)
  {
    m_dirtyFlags &= ~bit;
  }

  void set(TextParagraphCacheDirtyFlags bit)
  {
    m_dirtyFlags |= bit;
  }

  bool test(ETextParagraphCacheFlagsBits bit) const
  {
    return m_dirtyFlags & bit;
  }

  bool testAll(ETextParagraphCacheFlagsBits bit) const
  {
    return (m_dirtyFlags & bit) == bit;
  }

  void rebuild()
  {
    paragraphCache.clear();
    paragraphCache.reserve(paragraph.size());
    for (auto& d : paragraph)
    {
      assert(d.builder);
      paragraphCache.push_back(std::move(d.build()));
    }
    set(D_LAYOUT);
  }

  Bound layout(const Bound& bound, ETextLayoutMode mode)
  {
    Bound      newBound = bound;
    const auto layoutWidth = bound.width();
    m_height = 0;
    int newWidth = bound.width();
    int newHeight = bound.height();
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
        // TODO:: unexpected behavior occurs here.
        // A fixed number provided as a workaround temporarily.
        paragraph->layout(100000);
        const auto width = paragraph->getLongestLine();
        paragraph->layout(width);
      }
      else
      {
        paragraph->layout(layoutWidth - curX);
      }
      newWidth = std::max((int)paragraph->getLongestLine(), newWidth);
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
        m_height += paragraph->getHeight() - lineMetric.fHeight;
      }
      else
      {
        m_height += paragraph->getHeight();
      }
    }
    if (mode == ETextLayoutMode::TL_HeightAuto)
    {
      // update height only
      newBound.setHeight(newHeight);
    }
    if (mode == ETextLayoutMode::TL_WidthAuto)
    {
      newBound.setHeight(newHeight);
      newBound.setWidth(newWidth);
      // update both
    }
    return newBound;
  }

  int getHeight() const
  {
    return m_height;
  }
};

class TextNode__pImpl
{
  VGG_DECL_API(TextNode)
public:
  TextNode__pImpl(TextNode* api, const Bound& bound)
    : q_ptr(api)
    , paragraphCache(bound)
    , painter(nullptr, nullptr, Bound())
  {
  }

  std::string                text;
  TextParagraphCache         paragraphCache;
  ETextLayoutMode            mode;
  std::vector<TextStyleAttr> textAttr;
  VParagraphPainter          painter;
  ETextVerticalAlignment     vertAlign{ ETextVerticalAlignment::VA_Top };

  TextNode__pImpl(const TextNode__pImpl& p)
    : paragraphCache(Bound())
    , painter(nullptr, nullptr, Bound())
  {
    this->operator=(p);
  }

  TextNode__pImpl& operator=(const TextNode__pImpl& p)
  {
    text = p.text;
    mode = p.mode;
    vertAlign = p.vertAlign;
    // do not copy cache
    return *this;
  }

  TextNode__pImpl(TextNode__pImpl&& p) noexcept = default;
  TextNode__pImpl& operator=(TextNode__pImpl&& p) noexcept = default;
};
} // namespace VGG::layer
