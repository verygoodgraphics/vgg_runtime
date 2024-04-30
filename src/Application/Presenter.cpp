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
#include "Layer/Model/StructModel.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

#include <algorithm>
#include <unordered_map>

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG;

const auto K_EDITOR_PADDING = 100;

void Presenter::fitForEditing(const Layout::Size& pageSize)
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
  m_view->setOffsetAndScale(xOffset, yOffset, scale);
  m_view->enableZoomer(true);
  m_view->setScrollEnabled(false);
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
          DEBUG(
            "set cursor, target = %s, event = %s, shouldHandleHover = %d",
            targetKey.c_str(),
            uiEventTypeToString(EUIEventType::CLICK),
            shouldHandleHover);
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

  std::vector<layer::StructFrameObject> frames;
  for (auto& f : *viewModel->designDoc())
  {
    if (f->type() == VGG::Domain::Element::EType::FRAME)
    {
      frames.emplace_back(layer::StructFrameObject(f.get()));
    }
  }

  std::unordered_map<std::string, FontInfo> requiredFonts;
  auto                                      result =
    layer::SceneBuilder::builder()
      .setResetOriginEnable(true)
      .setFontNameVisitor(
        [&requiredFonts](const std::string& familyName, const std::string& subfamilyName) {
          requiredFonts[familyName + subfamilyName] = FontInfo{ familyName, subfamilyName };
        })
      .build<layer::StructModelFrame>(std::move(frames));

  if (result.root)
  {
    m_view->show(m_viewModel, std::move(*result.root));
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
    m_view->show(m_viewModel, true);
  }
}

bool Presenter::presentPage(int index)
{
  if (m_view)
  {
    return m_view->presentPage(index);
  }
  return false;
}

bool Presenter::dismissPage()
{
  if (m_view)
  {
    return m_view->dismissPage();
  }
  return false;
}

bool Presenter::goBack(bool resetScrollPosition, bool resetState)
{
  ASSERT(m_view);
  return m_view->goBack(resetScrollPosition, resetState);
}

void Presenter::saveState(std::shared_ptr<StateTree> stateTree)
{
  ASSERT(m_view);
  m_view->saveState(stateTree);
}

std::shared_ptr<StateTree> Presenter::savedState()
{
  ASSERT(m_view);
  return m_view->savedState();
}

void Presenter::restoreState()
{
  ASSERT(m_view);
  m_view->restoreState();
}

void Presenter::triggerMouseEnter()
{
  ASSERT(m_view);
  m_view->triggerMouseEnter();
}

void Presenter::fitForRunning(const Layout::Size& pageSize)
{
  if (!m_view)
  {
    DEBUG("Presenter::fitForRunning return");
    return;
  }

  DEBUG("Presenter::fitForRunning: contentMode = %d", m_contentMode);
  switch (m_contentMode)
  {
    case EContentMode::TOP_LEFT:
      m_view->resetOffsetAndScale();
      m_view->setContentSize(pageSize);
      break;
    case EContentMode::SCALE_ASPECT_FILL:
    case EContentMode::SCALE_ASPECT_FILL_TOP_CENTER:
    case EContentMode::SCALE_ASPECT_FIT:
      fitForAspectScale(pageSize);
      break;
  }

  m_view->enableZoomer(false);
  m_view->setScrollEnabled(true);
}

void Presenter::fitForAspectScale(const Layout::Size& pageSize)
{
  if (!m_view)
  {
    return;
  }

  auto         scale = 1.0;
  Layout::Size contentSize;
  const auto   maxSize = viewSize();
  if (
    !doublesNearlyEqual(pageSize.width, maxSize.width) ||
    !doublesNearlyEqual(pageSize.height, maxSize.height))
  {
    // scale
    const auto xScale = maxSize.width / pageSize.width;
    const auto yScale = maxSize.height / pageSize.height;
    switch (m_contentMode)
    {
      case EContentMode::SCALE_ASPECT_FILL:
      case EContentMode::SCALE_ASPECT_FILL_TOP_CENTER:
        scale = std::max(xScale, yScale);
        break;
      case EContentMode::SCALE_ASPECT_FIT:
        scale = std::min(xScale, yScale);
        break;
      case EContentMode::TOP_LEFT:
        return;
    }

    contentSize.width = pageSize.width * scale;
    contentSize.height = pageSize.height * scale;
  }
  else
  {
    contentSize = pageSize;
  }

  if (contentSize.width > maxSize.width || contentSize.height > maxSize.height)
  {
    m_view->setContentSize(contentSize);
  }

  const auto xOffset = (maxSize.width - contentSize.width) / 2;
  const auto yOffset = m_contentMode == EContentMode::SCALE_ASPECT_FILL_TOP_CENTER
                         ? 0
                         : (maxSize.height - contentSize.height) / 2;

  // set offset last, setContentSize may change contentOffset
  DEBUG(
    "Presenter::fitForAspectScale: xOffset = %f, yOffset = %f, scale = %f",
    xOffset,
    yOffset,
    scale);
  m_view->setOffsetAndScale(xOffset, yOffset, scale);
}

bool Presenter::setCurrentPage(std::size_t index, bool animated)
{
  return m_view->setCurrentPageIndex(index, animated);
}

bool Presenter::setCurrentPage(std::size_t index)
{
  return setCurrentPage(index, false);
}

bool Presenter::setCurrentPageIndex(
  std::size_t                   index,
  const app::UIAnimationOption& option,
  app::AnimationCompletion      completion)
{
  return m_view->setCurrentPageIndex(index, option, completion);
}