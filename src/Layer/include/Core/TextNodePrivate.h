#include "Core/Attrs.h"
#include "Core/FontManager.h"
#include "Core/TextNode.h"
#include "Core/VGGType.h"
#include "SkiaBackend/SkiaImpl.h"
#include "Common/DebugCanvas.h"
#include "Core/ParagraphParser.h"
#include <core/SkColor.h>
#include <core/SkFontStyle.h>
#include <core/SkScalar.h>
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

using namespace skia::textlayout;

struct CursorState
{
  SkScalar cursorX = 0.f;
  SkScalar cursorY = 0.f;
  SkScalar layoutWidth = 500.f;

  void advanceX(SkScalar a)
  {
    cursorX += a;
    layoutWidth -= a;
  }
  void advanceY(SkScalar a)
  {
    cursorY += a;
  }

  void reset(SkScalar layout)
  {
    cursorY = 0;
    cursorX = 0;
    layoutWidth = layout;
  }
};

inline void drawParagraphDebugInfo(DebugCanvas& canvas,
                                   const TextParagraph& textParagraph,
                                   Paragraph* p,
                                   CursorState& state,
                                   int index)
{
  static SkColor colorTable[9] = {
    SK_ColorBLUE,   SK_ColorGREEN,  SK_ColorRED,  SK_ColorCYAN,   SK_ColorMAGENTA,
    SK_ColorYELLOW, SK_ColorDKGRAY, SK_ColorGRAY, SK_ColorLTGRAY,
  };
  canvas.get()->save();
  canvas.get()->translate(state.cursorX, state.cursorY);
  auto rects = p->getRectsForRange(0, 1000, RectHeightStyle::kMax, RectWidthStyle::kMax);
  auto h = p->getHeight();
  auto mw = p->getMaxWidth();
  auto iw = p->getMaxIntrinsicWidth();
  SkPaint pen;
  SkColor color = colorTable[index % 9];
  pen.setColor(SkColorSetA(color, 0x88));
  canvas.get()->drawRect(SkRect{ 0, 0, mw, h }, pen);
  // pen.setColor(SK_ColorBLUE);
  // canvas.get()->drawRect(SkRect{0, 0, iw, h}, pen);
  canvas.drawRects(color, rects);
  canvas.get()->restore();
}

class TextParagraphCache : public ParagraphListener
{
  std::vector<TextParagraph> paragraph;
  std::vector<std::unique_ptr<skia::textlayout::Paragraph>> paragraphCache;
  bool newParagraph{ true };
  ParagraphAttr paraAttr;
  bool m_dirty{ true };
  void clear()
  {
    m_dirty = false;
  }

  void markDirty()
  {
    m_dirty = true;
  }

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
  TextParagraphCache() = default;

  bool isDirty() const
  {
    return m_dirty;
  }

  void cache()
  {
    if (!isDirty())
      return;
    paragraphCache.clear();
    paragraphCache.reserve(paragraph.size());
    for (auto& d : paragraph)
    {
      paragraphCache.push_back(std::move(d.builder->Build()));
    }
    clear();
  }

  void drawParagraph(SkCanvas* canvas, const Bound2& bound)
  {
    if (paragraph.empty())
      return;
    cache();
    CursorState cursor;
    const auto layoutWidth = bound.width();
    cursor.reset(layoutWidth);
    DebugCanvas debugCanvas(canvas);
    int width = 0, height = 0;
    for (int i = 0; i < paragraphCache.size(); i++)
    {
      auto& p = paragraphCache[i];
      cursor.cursorX = paragraph[i].level;
      p->layout(cursor.layoutWidth - cursor.cursorX);
      p->paint(canvas, cursor.cursorX, cursor.cursorY);
      auto lastLine = p->lineNumber();
      if (lastLine < 1)
        continue;
      LineMetrics lineMetric;
      p->getLineMetricsAt(lastLine - 1, &lineMetric);
      const auto height = p->getHeight() - lineMetric.fHeight;
      if (Scene::isEnableDrawDebugBound())
      {
        drawParagraphDebugInfo(debugCanvas, paragraph[i], p.get(), cursor, i);
      }
      cursor.advanceY(height);
      // cursor.cursorX = width;
    }

    // update bound
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
  std::vector<TextStyleStub> styles;
  TextParagraphCache m_paragraphCache;
  CursorState m_cursorState;
  ETextLayoutMode mode;
  ETextVerticalAlignment m_vertAlign{ ETextVerticalAlignment::VA_Top };
};
} // namespace VGG
