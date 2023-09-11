#include "Editor.hpp"

#include <Application/UIView.hpp>
#include <Log.h>
#include <Scene/Zoomer.h>

#include <include/core/SkCanvas.h>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

constexpr auto K_MOUSE_CONTAINER_WIDTH = 30.0f;

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

  // canvas->save();
  // canvas->translate(rect.left(), rect.top());
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

  // canvas->restore();
}
} // namespace

void Editor::handleUIEvent(UIEventPtr event, std::weak_ptr<LayoutNode> targetNode)
{
  DEBUG("Editor::handleUIEvent: type = %s, target = %s",
        event->type().c_str(),
        event->path().c_str());

  switch (event->enumType())
  {
    case UIEventType::mousedown:
    {
      m_isMouseDown = true;

      checkMouseDownPostion(static_cast<MouseEvent*>(event.get()));
      if (m_mouseDownPostion != EFramePosition::NONE)
      {
        return;
      }

      if (auto contentView = m_contentView.lock())
      {
        if (targetNode.lock() == contentView->currentPage())
        {
          m_selectedNode.reset();
          return;
        }
      }

      m_selectedNode = targetNode;
      checkMouseDownPostion(static_cast<MouseEvent*>(event.get()));

      // todo, multiple selection, deselect
      break;
    }

    case UIEventType::mousemove:
    {
      resizeNode(static_cast<MouseEvent*>(event.get()));
      break;
    }

    case UIEventType::mouseup:
    {
      m_isMouseDown = false;
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

  auto contentView = m_contentView.lock();
  if (!contentView)
  {
    return;
  }

  canvas->save();

  auto offset = contentView->zoomer()->translate();
  auto zoom = contentView->zoomer()->scale();
  canvas->translate(offset.x, offset.y);
  canvas->scale(zoom, zoom);

  if (auto node = m_selectedNode.lock())
  {
    SkRect rect = toSkRect(node->frameToAncestor(contentView->currentPage()));
    drawFrame(canvas, rect);
    // todo, translate, scale
  }

  canvas->restore();
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

void Editor::checkMouseDownPostion(MouseEvent* mouseDown)
{
  m_mouseDownPostion = EFramePosition::NONE;

  auto selectedNode = m_selectedNode.lock();
  if (!selectedNode)
  {
    return;
  }

  auto contentView = m_contentView.lock();
  if (!contentView)
  {
    return;
  }

  Layout::Point mouse{ TO_VGG_LAYOUT_SCALAR(mouseDown->x), TO_VGG_LAYOUT_SCALAR(mouseDown->y) };
  auto f = selectedNode->frameToAncestor(contentView->currentPage());

  DEBUG("Editor::checkMouseDownPostion: selected node frame is: (%f, %f, %f, %f), mouse at: %f, %f",
        f.origin.x,
        f.origin.y,
        f.size.width,
        f.size.height,
        mouse.x,
        mouse.y);

  // top corner
  Layout::Rect topLeftCorner{ { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                                f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
                              { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
  if (topLeftCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click top left corner");
    m_mouseDownPostion = EFramePosition::TOP_LEFT;
    return;
  }

  Layout::Rect topRightCorner{ { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
                               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
  if (topRightCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click top right corner");
    m_mouseDownPostion = EFramePosition::TOP_RIGHT;
    return;
  }

  // bottom corner
  Layout::Rect bottomLeftCorner{ { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                                   f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
                                 { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
  if (bottomLeftCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click bottom left corner");
    m_mouseDownPostion = EFramePosition::BOTTOM_LEFT;
    return;
  }

  Layout::Rect bottomRightCorner{ { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                                    f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
                                  { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
  if (bottomRightCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click bottom right corner");
    m_mouseDownPostion = EFramePosition::BOTTOM_RIGHT;
    return;
  }

  // top/bottom border
  Layout::Rect topBorder{ { f.origin.x + K_MOUSE_CONTAINER_WIDTH / 2,
                            f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
                          { f.size.width - K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
  if (topBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click top border");
    m_mouseDownPostion = EFramePosition::TOP;
    return;
  }

  Layout::Rect bottomBorder{ { f.origin.x + K_MOUSE_CONTAINER_WIDTH / 2,
                               f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
                             { f.size.width - K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
  if (bottomBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click bottom border");
    m_mouseDownPostion = EFramePosition::BOTTOM;
    return;
  }

  // left/right border
  Layout::Rect leftBorder{ { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                             f.origin.y + K_MOUSE_CONTAINER_WIDTH / 2 },
                           { K_MOUSE_CONTAINER_WIDTH, f.size.height - K_MOUSE_CONTAINER_WIDTH } };
  if (leftBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click left border");
    m_mouseDownPostion = EFramePosition::LEFT;
    return;
  }

  Layout::Rect rightBorder{ { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                              f.origin.y + K_MOUSE_CONTAINER_WIDTH / 2 },
                            { K_MOUSE_CONTAINER_WIDTH, f.size.height - K_MOUSE_CONTAINER_WIDTH } };
  if (rightBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click right border");
    m_mouseDownPostion = EFramePosition::RIGHT;
    return;
  }
}

void Editor::resizeNode(MouseEvent* mouseMove)
{
  if (!m_isMouseDown)
  {
    return;
  }

  auto selectedNode = m_selectedNode.lock();
  if (!selectedNode)
  {
    return;
  }

  if (m_mouseDownPostion == EFramePosition::NONE)
  {
    return;
  }

  auto frame = selectedNode->frame();
  auto tx = mouseMove->movementX;
  auto ty = mouseMove->movementY;

  DEBUG("Editor::resizeNode: selected node frame is: (%f, %f, %f, %f), mouse move is: %d, %d",
        frame.origin.x,
        frame.origin.y,
        frame.size.width,
        frame.size.height,
        tx,
        ty);

  switch (m_mouseDownPostion)
  {
    case EFramePosition::TOP_LEFT:
    {
      frame.origin.x += tx;
      frame.origin.y += ty;

      frame.size.width -= tx;
      frame.size.height -= ty;
      break;
    }
    case EFramePosition::TOP_RIGHT:
    {
      frame.origin.y += ty;

      frame.size.width += tx;
      frame.size.height -= ty;
      break;
    }

    case EFramePosition::BOTTOM_LEFT:
    {
      frame.origin.x += tx;

      frame.size.width -= tx;
      frame.size.height += ty;
      break;
    }
    case EFramePosition::BOTTOM_RIGHT:
    {
      frame.size.width += tx;
      frame.size.height += ty;
      break;
    }

    case EFramePosition::LEFT:
    {
      frame.origin.x += tx;

      frame.size.width -= tx;
      break;
    }
    case EFramePosition::RIGHT:
    {
      frame.size.width += tx;
      break;
    }

    case EFramePosition::TOP:
    {
      frame.origin.y += ty;

      frame.size.height -= ty;
      break;
    }
    case EFramePosition::BOTTOM:
    {
      frame.size.height += ty;
      break;
    }

    default:
      return;
  }

  selectedNode->setFrame(frame);
}