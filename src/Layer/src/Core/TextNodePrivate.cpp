#include "Core/TextNodePrivate.h"
#include "Core/FontManager.h"
#include "Core/TextNode.h"

#include <core/SkCanvas.h>
#include <core/SkColor.h>

namespace VGG
{

void drawParagraphDebugInfo(DebugCanvas& canvas, Paragraph* p, CursorState& state, int index)
{

  canvas.get()->save();
  canvas.get()->translate(state.cursorX, state.cursorY);
  auto rects = p->getRectsForRange(0, 10000, RectHeightStyle::kMax, RectWidthStyle::kMax);
  auto h = p->getHeight();
  auto mw = p->getMaxWidth();
  auto iw = p->getMaxIntrinsicWidth();
  SkPaint pen;
  pen.setColor(SkColorSetARGB(0xff, index * 0x33, index * 0x15, index * 0x05));
  canvas.get()->drawRect(SkRect{ 0, 0, mw, h }, pen);
  // pen.setColor(SK_ColorBLUE);
  // canvas.get()->drawRect(SkRect{0, 0, iw, h}, pen);
  canvas.drawRects(SK_ColorBLUE, rects);
  canvas.get()->restore();
  // p->paint(canvas.get(), state.cursorX, state.cursorY);
}

void TextNode__pImpl::drawParagraph(SkCanvas* canvas)

{
  // const auto& b = q_ptr->getBound();
  // const auto layoutWidht = b.width();
  // m_cursorState.reset(layoutWidht);
  //
  // DebugCanvas debugCanvas(canvas);
  // int count = 0;
  // for (auto& p : m_paragraphs)
  // {
  //   p->layout(m_cursorState.layoutWidth);
  //   const auto height = p->getHeight();
  //   p->paint(canvas, m_cursorState.cursorX, m_cursorState.cursorY);
  //   if (Scene::isEnableDrawDebugBound())
  //   {
  //     drawParagraphDebugInfo(debugCanvas, p.get(), m_cursorState, count);
  //   }
  //   m_cursorState.advanceY(height);
  //   count++;
  // }
}
} // namespace VGG
