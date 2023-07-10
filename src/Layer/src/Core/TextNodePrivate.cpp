#include "Core/TextNodePrivate.h"
#include "Core/FontManager.h"
#include "Core/TextNode.h"

#include <core/SkCanvas.h>
#include <core/SkColor.h>

namespace VGG
{

void drawParagraphDebugInfo(DebugCanvas& canvas, Paragraph* p, CursorState& state)
{

  canvas.get()->save();
  canvas.get()->translate(state.cursorX, state.cursorY);
  auto rects = p->getRectsForRange(0, 100, RectHeightStyle::kMax, RectWidthStyle::kMax);
  auto h = p->getHeight();
  auto mw = p->getMaxWidth();
  auto iw = p->getMaxIntrinsicWidth();
  SkPaint pen;
  pen.setColor(SK_ColorGREEN);
  canvas.get()->drawRect(SkRect{ 0, 0, mw, h }, pen);
  // pen.setColor(SK_ColorBLUE);
  // canvas.get()->drawRect(SkRect{0, 0, iw, h}, pen);
  canvas.drawRects(SK_ColorRED, rects);
  canvas.get()->restore();
  p->paint(canvas.get(), state.cursorX, state.cursorY);
}

void TextNode__pImpl::drawParagraph(SkCanvas* canvas)

{
  assert(m_paragraphSet);
  if (m_paragraphs.empty())
  {
    for (auto& pb : m_paragraphSet->Paragraphs)
    {
      m_paragraphs.push_back(std::move(pb.builder->Build()));
    }
  }
  const auto& b = q_ptr->getBound();
  const auto layoutWidht = b.width();
  m_cursorState.reset(layoutWidht);
  for (auto& p : m_paragraphs)
  {
    p->layout(m_cursorState.layoutWidth);
    const auto height = p->getHeight();
    p->paint(canvas, m_cursorState.cursorX, m_cursorState.cursorY);
    m_cursorState.advanceY(height);
  }
}
} // namespace VGG
