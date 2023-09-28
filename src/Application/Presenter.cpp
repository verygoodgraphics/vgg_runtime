#include "Presenter.hpp"

#include "Mouse.hpp"

#include "Utility/Log.hpp"

#include <algorithm>

using namespace VGG;

const auto K_EDITOR_PADDING = 100;

void Presenter::fitForEditing(Layout::Size pageSize)
{
  auto maxSize = viewSize();
  maxSize.width -= 2 * K_EDITOR_PADDING;
  maxSize.height -= 2 * K_EDITOR_PADDING;

  auto scale = 1.0;
  Layout::Size contentSize;
  if (pageSize.width > maxSize.width || pageSize.height > maxSize.height)
  {
    // scale
    auto xScale = maxSize.width / pageSize.width;
    auto yScale = maxSize.height / pageSize.height;
    scale = std::min(xScale, yScale);

    contentSize.width = pageSize.width * scale;
    contentSize.height = pageSize.height * scale;
  }
  else
  {
    contentSize = pageSize;
  }

  auto xOffset = (maxSize.width - contentSize.width) / 2 + K_EDITOR_PADDING;
  auto yOffset = (maxSize.height - contentSize.height) / 2 + K_EDITOR_PADDING;

  ASSERT(m_view);
  m_view->fitContent(xOffset, yOffset, scale);
}

void Presenter::resetForRunning()
{
  ASSERT(m_view);
  m_view->fitContent(0, 0, 1);
}

void Presenter::listenViewEvent()
{
  auto weakThis = weak_from_this();
  m_view->registerEventListener(
    [weakThis](std::string path, EUIEventType eventType)
    {
      auto sharedThis = weakThis.lock();
      if (!sharedThis)
      {
        return false;
      }

      if (sharedThis->m_editMode)
      {
        return true;
      }

      auto listenersMap = sharedThis->m_viewModel->model->getEventListeners(path);
      std::string type = uiEventTypeToString(eventType);

      auto hasUserListener = listenersMap.find(type) != listenersMap.end();
      if (hasUserListener)
      {
        return true;
      }

      if (eventType == EUIEventType::MOUSEMOVE)
      {
        // process hover
        auto shouldHandleHover =
          listenersMap.find(uiEventTypeToString(EUIEventType::CLICK)) != listenersMap.end() ||
          listenersMap.find(uiEventTypeToString(EUIEventType::MOUSEDOWN)) != listenersMap.end() ||
          listenersMap.find(uiEventTypeToString(EUIEventType::MOUSEUP)) != listenersMap.end();
        if (auto mouse = sharedThis->m_mouse)
        {
          mouse->setCursor(shouldHandleHover ? Mouse::ECursor::HAND : Mouse::ECursor::ARROW);
        }
        if (shouldHandleHover)
        {
          return true; // stop hit test
        }
      }

      return false;
    });
}