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
#include "Presenter.hpp"

#include "Mouse.hpp"

#include "Domain/Layout/Node.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

#include <algorithm>
#include <unordered_map>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

const auto K_EDITOR_PADDING = 100;

void Presenter::fitForEditing(Layout::Size pageSize)
{
  auto maxSize = viewSize();
  maxSize.width -= 2 * K_EDITOR_PADDING;
  maxSize.height -= 2 * K_EDITOR_PADDING;

  auto         scale = 1.0;
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

  if (!m_view)
  {
    return;
  }
  m_view->fitContent(xOffset, yOffset, scale);
  m_view->enableZoomer(true);
  m_view->setScrollEnabled(false);
}

void Presenter::resetForRunning()
{
  if (!m_view)
  {
    return;
  }
  m_view->fitCurrentPage();
  m_view->enableZoomer(false);
  m_view->setScrollEnabled(true);
}

void Presenter::listenViewEvent()
{
  auto weakThis = weak_from_this();
  m_view->registerEventListener(
    [weakThis](std::string targetKey, EUIEventType eventType)
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

      auto sharedModel = sharedThis->m_viewModel->model.lock();
      if (!sharedModel)
      {
        return false;
      }

      auto        listenersMap = sharedModel->getEventListeners(targetKey);
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

      if (sharedThis->m_listenAllEvents)
      {
        return true;
      }

      return false;
    });
}

void Presenter::setView(std::shared_ptr<UIScrollView> view)
{
  m_view = view;

  if (!m_view)
  {
    WARN("#Presenter::setView, null m_view, return");
    return;
  }

  auto weakThis = weak_from_this();
  m_view->setEventListener(
    [weakThis](UIEventPtr evtPtr, std::weak_ptr<LayoutNode> targetNode)
    {
      if (auto sharedThis = weakThis.lock())
      {
        if (sharedThis->m_editMode)
        {
          sharedThis->m_editorEventListener(evtPtr, targetNode);
        }
        else if (sharedThis->m_listenAllEvents)
        {
          sharedThis->m_subject.get_subscriber().on_next(evtPtr);
        }
        else if (!targetNode.expired())
        {
          sharedThis->m_subject.get_subscriber().on_next(evtPtr);
        }
      }
    });
}

bool Presenter::handleTranslate(double pageWidth, double pageHeight, float x, float y)
{
  const auto [_, viewHeight] = viewSize();
  if (viewHeight >= pageHeight)
  {
    return false;
  }

  // vertical scoll
  const auto oldOffset = m_view->contentOffset();
  auto       newOffset = oldOffset;
  newOffset.y += y;
  // limit offset range
  newOffset.y = std::max(viewHeight - pageHeight, newOffset.y);
  newOffset.y = std::min(newOffset.y, 0.0);

  if (doublesNearlyEqual(newOffset.y, oldOffset.y))
  {
    return false;
  }
  else
  {
    DEBUG(
      "Presenter::handleTranslate: page size: %f, %f, scroll, %f, %f; old offset is: %f, %f, new "
      "offset is: %f, %f",
      pageWidth,
      pageHeight,
      x,
      y,
      oldOffset.x,
      oldOffset.y,
      newOffset.x,
      newOffset.y);
    m_view->setContentOffset(newOffset, true);
    return true;
  }
}

void Presenter::setModel(std::shared_ptr<ViewModel> viewModel)
{
  m_viewModel = viewModel;

  if (!m_view)
  {
    WARN("#Presenter::setModel, null m_view, return");
    return;
  }

  std::unordered_map<std::string, FontInfo> requiredFonts;
  auto                                      result =
    layer::SceneBuilder::builder()
      .setResetOriginEnable(true)
      .setFontNameVisitor(
        [&requiredFonts](const std::string& familyName, const std::string& subfamilyName) {
          requiredFonts[familyName + subfamilyName] = FontInfo{ familyName, subfamilyName };
        })
      .build(viewModel->designDoc()->content());

  if (result.root)
  {
    m_view->show(*m_viewModel, std::move(*result.root));
  }
  else
  {
    WARN("#Presenter::setModel, built scene is empty , return");
    return;
  }

  for (auto kv : requiredFonts)
  {
    m_requiredFonts.push_back(kv.second);
  }

  listenViewEvent();
}

void Presenter::update()
{
  if (m_view && m_viewModel)
  {
    m_viewModel->layoutTree()->layoutIfNeeded();
    m_view->show(*m_viewModel, true);
  }
}
