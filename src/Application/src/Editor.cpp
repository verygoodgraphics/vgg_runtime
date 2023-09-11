#include "Editor.hpp"

#include <Log.h>

#include <include/core/SkCanvas.h>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

namespace
{
SkRect toSkRect(const Layout::Rect rect)
{
  return { rect.origin.x,
           rect.origin.y,
           rect.origin.x + rect.size.width,
           rect.origin.y + rect.size.height };
}

void drawFrame(SkCanvas* canvas, const SkRect& rect)
{
  ASSERT(canvas);
  ASSERT(rect.right() > rect.left());
  ASSERT(rect.bottom() > rect.top());

  canvas->save();
  canvas->translate(rect.left(), rect.top());
  // todo, flip
  // todo, rotate

  // todo, hover border
  // if (rc == RenderCase::RC_HOVER)
  // {
  //   SkPaint strokePen;
  //   strokePen.setAntiAlias(true);
  //   strokePen.setStyle(SkPaint::kStroke_Style);
  //   strokePen.setColor(SK_ColorBLUE);
  //   strokePen.setStrokeWidth(2);
  //   canvas->drawRect(f.toLocalSkRect(), strokePen);

  //   canvas->restore();
  //   return;
  // }
  // else if (rc == RenderCase::RC_EDIT)

  SkPaint strokePen;
  strokePen.setAntiAlias(true);
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setColor(SK_ColorBLUE);
  canvas->drawRect(rect, strokePen);

  canvas->restore();
}
} // namespace

void Editor::handleUIEvent(UIEventPtr event, std::weak_ptr<LayoutNode> targetNode)
{
  DEBUG("Editor::handleUIEvent: type = %s, target = %s",
        event->type().c_str(),
        event->path().c_str());

  switch (event->enumType())
  {
    case UIEventType::click:
    {
      m_selectedNode = targetNode;
      // todo, multiple selection, deselect
      break;
    }

    default:
      return;
  }
}

void Editor::onRender(SkCanvas* canvas)
{
  if (!m_enabled)
  {
    return;
  }

  DEBUG("Editor::onRender");

  if (auto node = m_selectedNode.lock())
  {
    SkRect rect = toSkRect(node->frame());
    drawFrame(canvas, rect);
    // todo, translate, scale
  }
}

void Editor::drawBorder(SkCanvas* canvas, const LayoutNode* node)
{
  ASSERT(canvas);
  auto frame = node->frame();
  ASSERT(frame.size.width > 0);
  ASSERT(frame.size.height > 0);
}

void Editor::drawCornerPoint(SkCanvas* canvas, const LayoutNode* node)
{
  // ASSERT(canvas);
  // ASSERT(f.w > 0);
  // ASSERT(f.h > 0);

  // // todo, align line
  // if (fe.draggingFrame && !fe.currEditAnchor.has_value())
  // {
  //   return;
  // }

  // canvas->save();
  // canvas->translate(f.x, f.y);
  // canvas->translate(f.w / 2, f.h / 2);
  // canvas->scale(f.flipX ? -1 : 1, f.flipY ? -1 : 1);
  // canvas->rotate(f.rotation);
  // canvas->translate(-f.w / 2, -f.h / 2);

  // SkPaint strokePen;
  // strokePen.setStyle(SkPaint::kStroke_Style);
  // strokePen.setColor(SK_ColorBLACK);

  // SkPaint fillPen;
  // fillPen.setStyle(SkPaint::kFill_Style);
  // fillPen.setColor(SK_ColorWHITE);

  // SkPaint fillPen2;
  // fillPen2.setStyle(SkPaint::kFill_Style);
  // fillPen2.setColor(SK_ColorBLUE);

  // double dw = f.w / 2;
  // double dh = f.h / 2;
  // double cx = dw;
  // double cy = dh;
  // for (int i = -1; i <= 1; i++)
  //   for (int j = -1; j <= 1; j++)
  //   {
  //     int idx = i * 3 + j;
  //     if (idx == 0)
  //     {
  //       continue;
  //     }
  //     if (fe.draggingFrame && fe.currEditAnchor.has_value() && fe.currEditAnchor != idx)
  //     {
  //       continue;
  //     }
  //     SkRect ctrPt;
  //     ctrPt.setXYWH(cx + i * dw - CurvePoint::SIZE,
  //                   cy + j * dh - CurvePoint::SIZE,
  //                   CurvePoint::SIZE * 2,
  //                   CurvePoint::SIZE * 2);
  //     canvas->drawRect(ctrPt, idx == fe.currEditAnchor ? fillPen2 : fillPen);
  //     canvas->drawRect(ctrPt, strokePen);
  //   }
  // canvas->restore();
}