/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Editor.hpp"
#include <algorithm>
#include "Application/UIView.hpp"
#include "Domain/Layout/LayoutNode.hpp"
#include "Event/Keycode.hpp"
#include "Mouse.hpp"
#include "Utility/Log.hpp"
#include <core/SkColor.h>
#include <core/SkPaint.h>
#include <core/SkRect.h>
#include <core/SkScalar.h>
#include <glm/detail/qualifier.hpp>
#include <include/core/SkCanvas.h>

#undef INFO
#define INFO(msg, ...)

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

constexpr auto K_MOUSE_CONTAINER_WIDTH = 10.0;
constexpr auto K_RESIZE_MIN_LENGTH = 1.0;

namespace
{
SkRect toSkRect(const Layout::Rect rect)
{
  return { SkDoubleToScalar(rect.origin.x),
           SkDoubleToScalar(rect.origin.y),
           SkDoubleToScalar(rect.origin.x + rect.size.width),
           SkDoubleToScalar(rect.origin.y + rect.size.height) };
}

void drawFrame(SkCanvas* canvas, const SkRect& rect, const SkScalar strokeWidth = 1)
{
  ASSERT(canvas);
  ASSERT(rect.right() >= rect.left());
  ASSERT(rect.bottom() >= rect.top());

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
  DEBUG(
    "Editor::handleUIEvent: type = %s, target = %s",
    event->type().c_str(),
    event->targetName().c_str());

  switch (event->enumType())
  {
    case EUIEventType::MOUSEDOWN:
    {
      m_isMouseDown = true;

      auto mouseEvent = static_cast<MouseEvent*>(event.get());

      m_mouseDownPosition = checkMousePostion(mouseEvent->x, mouseEvent->y);
      if (m_mouseDownPosition != EResizePosition::NONE)
      {
        return;
      }

      if (m_selectedNode.lock() != targetNode.lock())
      {
        didSelectNode(targetNode);
        m_mouseDownPosition = checkMousePostion(mouseEvent->x, mouseEvent->y);
      }

      // todo, multiple selection, deselect
      break;
    }

    case EUIEventType::MOUSEMOVE:
    {
      auto mouseEvent = static_cast<MouseEvent*>(event.get());
      resizeNode(mouseEvent);

      auto position = checkMousePostion(mouseEvent->x, mouseEvent->y);
      if (position == EResizePosition::NONE && m_hoverNode.lock() != targetNode.lock())
      {
        m_hoverNode = targetNode;
        setDirty(true);
      }
      updateCursor(position);

      break;
    }

    case EUIEventType::MOUSEUP:
    {
      m_isMouseDown = false;
      break;
    }

    case EUIEventType::KEYDOWN:
    {
      auto keyEvent = static_cast<KeyboardEvent*>(event.get());
      if (keyEvent->key == ' ')
      {
        m_mouse->setCursor(Mouse::ECursor::HAND);
      }
      break;
    }

    case EUIEventType::KEYUP:
    {
      auto keyEvent = static_cast<KeyboardEvent*>(event.get());
      if (keyEvent->key == VGGK_SPACE)
      {
        m_mouse->resetCursor();
      }
      break;
    }

    default:
      return;
  }
}

void Editor::onRender(SkCanvas* canvas)
{
  if (!m_isEnabled)
  {
    return;
  }

  auto contentView = m_contentView.lock();
  if (!contentView)
  {
    return;
  }

  canvas->save();

  auto offset = contentView->offset();
  auto zoom = contentView->scale();
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
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EResizePosition::TOP_LEFT)));
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EResizePosition::TOP_RIGHT)));
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EResizePosition::BOTTOM_LEFT)));
    drawRectCorner(canvas, toSkRect(getSelectNodeRect(EResizePosition::BOTTOM_RIGHT)));
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

  setDirty(false);
}

void Editor::drawBorder(SkCanvas* canvas, const LayoutNode* node)
{
  ASSERT(canvas);
  auto frame = node->frame();
  ASSERT(frame.size.width > 0);
  ASSERT(frame.size.height > 0);
}

Editor::EResizePosition Editor::checkMousePostion(int x, int y)
{
  auto position = EResizePosition::NONE;

  auto selectedNode = m_selectedNode.lock();
  if (!selectedNode)
  {
    return position;
  }

  auto contentView = m_contentView.lock();
  if (!contentView)
  {
    return position;
  }

  Layout::Point mouse{ TO_VGG_LAYOUT_SCALAR(x), TO_VGG_LAYOUT_SCALAR(y) };
  Layout::Rect  f;
  if (selectedNode == contentView->currentPage())
  {
    f.size = selectedNode->frame().size;
  }
  else
  {
    f = selectedNode->frameToAncestor(contentView->currentPage());
  }

  DEBUG(
    "Editor::checkMousePostion: selected node frame is: (%f, %f, %f, %f), mouse at: %f, %f",
    f.origin.x,
    f.origin.y,
    f.size.width,
    f.size.height,
    mouse.x,
    mouse.y);

  // top corner
  Layout::Rect topLeftCorner{ getSelectNodeRect(EResizePosition::TOP_LEFT) };
  if (topLeftCorner.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at top left corner");
    position = EResizePosition::TOP_LEFT;
    return position;
  }

  Layout::Rect topRightCorner{ getSelectNodeRect(EResizePosition::TOP_RIGHT) };
  if (topRightCorner.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at top right corner");
    position = EResizePosition::TOP_RIGHT;
    return position;
  }

  // bottom corner
  Layout::Rect bottomLeftCorner{ getSelectNodeRect(EResizePosition::BOTTOM_LEFT) };
  if (bottomLeftCorner.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at bottom left corner");
    position = EResizePosition::BOTTOM_LEFT;
    return position;
  }

  Layout::Rect bottomRightCorner{ getSelectNodeRect(EResizePosition::BOTTOM_RIGHT) };
  if (bottomRightCorner.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at bottom right corner");
    position = EResizePosition::BOTTOM_RIGHT;
    return position;
  }

  // top/bottom border
  Layout::Rect topBorder{ getSelectNodeRect(EResizePosition::TOP) };
  if (topBorder.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at top border");

    position = EResizePosition::TOP;
    return position;
  }

  Layout::Rect bottomBorder{ getSelectNodeRect(EResizePosition::BOTTOM) };
  if (bottomBorder.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at bottom border");
    position = EResizePosition::BOTTOM;
    return position;
  }

  // left/right border
  Layout::Rect leftBorder{ getSelectNodeRect(EResizePosition::LEFT) };
  if (leftBorder.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at left border");
    position = EResizePosition::LEFT;
    return position;
  }

  Layout::Rect rightBorder{ getSelectNodeRect(EResizePosition::RIGHT) };
  if (rightBorder.contains(mouse))
  {
    DEBUG("Editor::checkMousePostion: at right border");
    position = EResizePosition::RIGHT;
    return position;
  }

  return position;
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

  if (m_mouseDownPosition == EResizePosition::NONE)
  {
    return;
  }

  auto contentView = m_contentView.lock();
  if (!contentView)
  {
    return;
  }

  auto frame = selectedNode->frame();

  const auto tx = mouseMove->movementX;
  const auto ty = mouseMove->movementY;
  const auto scale = contentView->scale();
  const auto dw = tx / scale;
  const auto dh = ty / scale;

  DEBUG(
    "Editor::resizeNode: selected node frame: (%f, %f, %f, %f), mouse move: %d, %d, dw: %f, dh: "
    "%f, scale: %f",
    frame.origin.x,
    frame.origin.y,
    frame.size.width,
    frame.size.height,
    tx,
    ty,
    dw,
    dh,
    scale);

  switch (m_mouseDownPosition)
  {
    case EResizePosition::TOP_LEFT:
    {
      frame.origin.x += tx;
      frame.origin.y += ty;

      frame.size.width -= dw;
      frame.size.height -= dh;
      break;
    }
    case EResizePosition::TOP_RIGHT:
    {
      frame.origin.y += ty;

      frame.size.width += dw;
      frame.size.height -= dh;
      break;
    }

    case EResizePosition::BOTTOM_LEFT:
    {
      frame.origin.x += tx;

      frame.size.width -= dw;
      frame.size.height += dh;
      break;
    }
    case EResizePosition::BOTTOM_RIGHT:
    {
      frame.size.width += dw;
      frame.size.height += dh;
      break;
    }

    case EResizePosition::LEFT:
    {
      frame.origin.x += tx;

      frame.size.width -= dw;
      break;
    }
    case EResizePosition::RIGHT:
    {
      frame.size.width += dw;
      break;
    }

    case EResizePosition::TOP:
    {
      frame.origin.y += ty;

      frame.size.height -= dh;
      break;
    }
    case EResizePosition::BOTTOM:
    {
      frame.size.height += dh;
      break;
    }

    default:
      return;
  }

  frame.size.width = std::max(frame.size.width, K_RESIZE_MIN_LENGTH);
  frame.size.height = std::max(frame.size.height, K_RESIZE_MIN_LENGTH);

  if (selectedNode == contentView->currentPage())
  {
    auto offset = frame.origin - selectedNode->frame().origin;
    switch (m_mouseDownPosition)
    {
      case EResizePosition::TOP_LEFT:
      case EResizePosition::TOP_RIGHT:
      case EResizePosition::BOTTOM_LEFT:
      case EResizePosition::TOP:
      case EResizePosition::LEFT:
        contentView->translate(offset.x, offset.y);
        break;
      default:
        break;
    }
  }
  selectedNode->setFrame(frame, true);
  m_isModelDirty = true;
}

Layout::Rect Editor::getSelectNodeRect(EResizePosition position)
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
    case EResizePosition::TOP_LEFT:
    {
      rect = { { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }
    case EResizePosition::TOP_RIGHT:
    {
      rect = { { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    case EResizePosition::BOTTOM_LEFT:
    {
      rect = { { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }
    case EResizePosition::BOTTOM_RIGHT:
    {
      rect = { { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + f.size.height - K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    case EResizePosition::LEFT:
    {
      rect = { { f.origin.x - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, f.size.height - K_MOUSE_CONTAINER_WIDTH } };

      break;
    }
    case EResizePosition::RIGHT:
    {
      rect = { { f.origin.x + f.size.width - K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y + K_MOUSE_CONTAINER_WIDTH / 2 },
               { K_MOUSE_CONTAINER_WIDTH, f.size.height - K_MOUSE_CONTAINER_WIDTH } };
      break;
    }

    case EResizePosition::TOP:
    {
      rect = { { f.origin.x + K_MOUSE_CONTAINER_WIDTH / 2,
                 f.origin.y - K_MOUSE_CONTAINER_WIDTH / 2 },
               { f.size.width - K_MOUSE_CONTAINER_WIDTH, K_MOUSE_CONTAINER_WIDTH } };
      break;
    }
    case EResizePosition::BOTTOM:
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

void Editor::enable(bool enabled)
{
  m_isEnabled = enabled;
  m_mouse->resetCursor();
}

void Editor::updateCursor(EResizePosition mousePosition)
{
  switch (mousePosition)
  {
    case EResizePosition::TOP:
    case EResizePosition::BOTTOM:
      m_mouse->setCursor(Mouse::ECursor::SIZENS);
      break;

    case EResizePosition::LEFT:
    case EResizePosition::RIGHT:
      m_mouse->setCursor(Mouse::ECursor::SIZEWE);
      break;

    case EResizePosition::TOP_LEFT:
    case EResizePosition::BOTTOM_RIGHT:
      m_mouse->setCursor(Mouse::ECursor::SIZENWSE);
      break;

    case EResizePosition::TOP_RIGHT:
    case EResizePosition::BOTTOM_LEFT:
      m_mouse->setCursor(Mouse::ECursor::SIZENESW);
      break;

    default:
      m_mouse->resetCursor();
      break;
  }
}

void Editor::didSelectNode(std::weak_ptr<LayoutNode> node)
{
  if (auto selectedNode = node.lock())
  {
    INFO("Editor::didSelectNode: id = %s", selectedNode->id().c_str());
  }

  m_selectedNode = node;
  setDirty(true);

  if (auto listener = m_listener.lock())
  {
    listener->onSelectNode(node);
  }
}

bool Editor::isDirty()
{
  return m_isEnabled && (m_isDirty || isModelDirty());
}
void Editor::setDirty(const bool dirty)
{
  m_isDirty = dirty;
}

bool Editor::isModelDirty()
{
  return m_isModelDirty;
}

void Editor::resetModelDirty()
{
  m_isModelDirty = false;
}
