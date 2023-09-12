#include "Editor.hpp"

#include <Application/UIView.hpp>
#include <Log.h>
#include <Scene/Zoomer.h>

#include <include/core/SkCanvas.h>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

constexpr auto K_MOUSE_CONTAINER_WIDTH = 10.0f;

namespace
{
SkRect toSkRect(const Layout::Rect rect)
{
  return { rect.origin.x,
           rect.origin.y,
           rect.origin.x + rect.size.width,
           rect.origin.y + rect.size.height };
}

void drawFrame(SkCanvas* canvas, const SkRect& rect, const SkScalar strokeWidth = 1)
{
  ASSERT(canvas);
  ASSERT(rect.right() > rect.left());
  ASSERT(rect.bottom() > rect.top());

  SkPaint strokePen;
  strokePen.setAntiAlias(true);
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setColor(SK_ColorBLUE);
  strokePen.setStrokeWidth(strokeWidth);
  canvas->drawRect(rect, strokePen);
}

void drawRectCorner(SkCanvas* canvas, const SkRect& rect)
{
  ASSERT(canvas);
  ASSERT(rect.right() > rect.left());
  ASSERT(rect.bottom() > rect.top());

  SkPaint strokePen;
  strokePen.setAntiAlias(true);
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setColor(SK_ColorBLUE);

  SkPaint fillPen;
  fillPen.setAntiAlias(true);
  fillPen.setStyle(SkPaint::kFill_Style);
  fillPen.setColor(SK_ColorWHITE);

  canvas->drawRect(rect, strokePen);
  canvas->drawRect(rect, fillPen);
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

      m_selectedNode = targetNode;
      checkMouseDownPostion(static_cast<MouseEvent*>(event.get()));

      // todo, multiple selection, deselect
      break;
    }

    case UIEventType::mousemove:
    {
      m_hoverNode = targetNode;
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

  auto selectedNode = m_selectedNode.lock();
  if (selectedNode)
  {
    SkRect rect;
    if (selectedNode == contentView->currentPage())
    {
      rect.fRight = selectedNode->frame().size.width;
      rect.fBottom = selectedNode->frame().size.height;
    }
    else
    {
      rect = toSkRect(selectedNode->frameToAncestor(contentView->currentPage()));
    }

    // todo, translate, scale
    drawFrame(canvas, rect);
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EFramePosition::TOP_LEFT)));
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EFramePosition::TOP_RIGHT)));
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EFramePosition::BOTTOM_LEFT)));
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EFramePosition::BOTTOM_RIGHT)));
  }

  if (auto node = m_hoverNode.lock(); node && node != selectedNode)
  {
    SkRect rect;
    if (node == contentView->currentPage())
    {
      rect.fRight = node->frame().size.width;
      rect.fBottom = node->frame().size.height;
    }
    else
    {
      rect = toSkRect(node->frameToAncestor(contentView->currentPage()));
    }

    // todo, translate, scale
    drawFrame(canvas, rect, 2.0f);
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
  Layout::Rect f;
  if (selectedNode == contentView->currentPage())
  {
    f.size = selectedNode->frame().size;
  }
  else
  {
    f = selectedNode->frameToAncestor(contentView->currentPage());
  }

  DEBUG("Editor::checkMouseDownPostion: selected node frame is: (%f, %f, %f, %f), mouse at: %f, %f",
        f.origin.x,
        f.origin.y,
        f.size.width,
        f.size.height,
        mouse.x,
        mouse.y);

  // top corner
  Layout::Rect topLeftCorner{ getSelectNodeRect(EFramePosition::TOP_LEFT) };
  if (topLeftCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click top left corner");
    m_mouseDownPostion = EFramePosition::TOP_LEFT;
    return;
  }

  Layout::Rect topRightCorner{ getSelectNodeRect(EFramePosition::TOP_RIGHT) };
  if (topRightCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click top right corner");
    m_mouseDownPostion = EFramePosition::TOP_RIGHT;
    return;
  }

  // bottom corner
  Layout::Rect bottomLeftCorner{ getSelectNodeRect(EFramePosition::BOTTOM_LEFT) };
  if (bottomLeftCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click bottom left corner");
    m_mouseDownPostion = EFramePosition::BOTTOM_LEFT;
    return;
  }

  Layout::Rect bottomRightCorner{ getSelectNodeRect(EFramePosition::BOTTOM_RIGHT) };
  if (bottomRightCorner.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click bottom right corner");
    m_mouseDownPostion = EFramePosition::BOTTOM_RIGHT;
    return;
  }

  // top/bottom border
  Layout::Rect topBorder{ getSelectNodeRect(EFramePosition::TOP) };
  if (topBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click top border");
    m_mouseDownPostion = EFramePosition::TOP;
    return;
  }

  Layout::Rect bottomBorder{ getSelectNodeRect(EFramePosition::BOTTOM) };
  if (bottomBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click bottom border");
    m_mouseDownPostion = EFramePosition::BOTTOM;
    return;
  }

  // left/right border
  Layout::Rect leftBorder{ getSelectNodeRect(EFramePosition::LEFT) };
  if (leftBorder.contains(mouse))
  {
    DEBUG("Editor::checkMouseDownPostion: click left border");
    m_mouseDownPostion = EFramePosition::LEFT;
    return;
  }

  Layout::Rect rightBorder{ getSelectNodeRect(EFramePosition::RIGHT) };
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

Layout::Rect Editor::getSelectNodeRect(EFramePosition position)
{
  Layout::Rect rect;

  auto selectedNode = m_selectedNode.lock();
  if (!selectedNode)
  {
    return rect;
  }

  auto contentView = m_contentView.lock();
  if (!contentView)
  {
    return rect;
  }

  Layout::Rect f;
  if (selectedNode == contentView->currentPage())
  {
    f.size = selectedNode->frame().size;
  }
  else
  {
    f = selectedNode->frameToAncestor(contentView->currentPage());
  }

  switch (position)
  {
    case EFramePosition::TOP_LEFT:
    {
      rect = { { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }
    case EFramePosition::TOP_RIGHT:
    {
      rect = { { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    case EFramePosition::BOTTOM_LEFT:
    {
      rect = { { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }
    case EFramePosition::BOTTOM_RIGHT:
    {
      rect = { { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    case EFramePosition::LEFT:
    {
      rect = { { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, f.size.height - K_MOUSE_CONTAINER_WIDTH } };

      break;
    }
    case EFramePosition::RIGHT:
    {
      rect = { { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, f.size.height - K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    case EFramePosition::TOP:
    {
      rect = { { f.origin.x + K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
               { f.size.width - K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }
    case EFramePosition::BOTTOM:
    {
      rect = { { f.origin.x + K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
               { f.size.width - K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    default:
      break;
  }

  return rect;
}