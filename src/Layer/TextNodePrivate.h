#include "VGG/Layer/Core/Attrs.h"
#include "VGG/Layer/FontManager.h"
#include "VGG/Layer/Core/TextNode.h"
#include "VGG/Layer/Core/VType.h"
#include "VSkia.h"
#include "VSkFontMgr.h"
#include "DebugCanvas.h"
#include "ParagraphParser.h"
#include <include/core/SkColor.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkRefCnt.h>
#include <include/core/SkScalar.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/Metrics.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>


namespace VGG
{

void drawParagraphDebugInfo(DebugCanvas& canvas,
                            const TextParagraph& textParagraph,
                            Paragraph* p,
                            int curX,
                            int curY,
                            int index);

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
  struct ParagraphInfo
  {
    int offsetX{ 0 };
    std::unique_ptr<sktxt::Paragraph> paragraph;
  };
  std::vector<ParagraphInfo> paragraphCache;

  void setFontCollection(sk_sp<VGGFontCollection> fontCollection)
  {
    this->m_fontCollection = std::move(fontCollection);
  }

private:
  int m_height{ 0 };
  bool m_newParagraph{ true };
  ParagraphAttr m_paraAttr;
  TextParagraphCacheDirtyFlags m_dirtyFlags{ D_ALL };
  sk_sp<VGGFontCollection> m_fontCollection;

protected:
  void onBegin() override;
  void onEnd() override;
  void onParagraphBegin(int paraIndex, int order, const ParagraphAttr& paragraAttr) override;
  void onParagraphEnd(int paraIndex, const TextView& textView) override;
  void onTextStyle(int paraIndex,
                   int styleIndex,
                   const TextView& textView,
                   const TextAttr& textAttr) override;

public:
  TextParagraphCache()
  {

    auto mgr = sk_sp<SkFontMgrVGG>(FontManager::instance().defaultFontManager());
    if (mgr)
    {
      mgr->ref();
    }
    m_fontCollection = sk_ref_sp(new VGGFontCollection(std::move(mgr)));
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
      assert(d.Builder);
      paragraphCache.push_back({ 0, std ::move(d.Builder->Build()) });
    }
    set(D_LAYOUT);
  }

  Bound2 layout(const Bound2& bound, ETextLayoutMode mode)
  {
    Bound2 newBound = bound;
    const auto layoutWidth = bound.width();
    m_height = 0;
    int newWidth = bound.width();
    int newHeight = bound.height();
    for (int i = 0; i < paragraphCache.size(); i++)
    {
      // auto paragraph = d.builder->Build();
      const auto& d = paragraph[i];
      const auto& paragraph = paragraphCache[i].paragraph;
      SkFontMetrics metrics;
      d.Builder->getParagraphStyle().getTextStyle().getFontMetrics(&metrics);
      const auto curX = metrics.fAvgCharWidth * d.Level;
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
      assert(!d.Utf8TextView.Text.empty());
      auto c = d.Utf8TextView.Text.back();
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
  TextNode__pImpl(TextNode* api)
    : q_ptr(api)
  {
  }

  std::string text;
  TextParagraphCache paragraphCache;
  ETextLayoutMode mode;
  ETextVerticalAlignment vertAlign{ ETextVerticalAlignment::VA_Top };

  TextNode__pImpl(const TextNode__pImpl& p)
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
} // namespace VGG
