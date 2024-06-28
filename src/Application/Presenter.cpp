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
#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_map>
#include "Application/AppLayoutContext.hpp"
#include "Application/ViewModel.hpp"
#include "Domain/Daruma.hpp"
#include "Domain/Layout/LayoutNode.hpp"
#include "Domain/Model/Element.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Mouse.hpp"
#include "UIAnimation.hpp"
#include "UIEvent.hpp"
#include "UIOptions.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"
#include <rxcpp/rx-includes.hpp>
#include <rxcpp/rx-predef.hpp>
#include <rxcpp/subjects/rx-subject.hpp>

#undef DEBUG
#define DEBUG(msg, ...)

namespace VGG
{

namespace
{
const auto K_EDITOR_PADDING = 100;
}

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
        return false;
      if (sharedThis->m_editMode)
        return true;

      auto sharedModel = sharedThis->m_viewModel->model.lock();
      if (!sharedModel)
        return false;

      auto        listenersMap = sharedModel->getEventListeners(targetKey);
      std::string type = uiEventTypeToString(eventType);
      if (listenersMap.find(type) != listenersMap.end()) // user listener
        return true;

      if (sharedThis->m_listenAllEvents)
        return true;

      return false;
    });

  m_view->setUpdateCursorEventListener(
    [weakThis](std::string targetKey, EUIEventType eventType)
    {
      if (eventType != EUIEventType::MOUSEMOVE)
        return false;
      auto sharedThis = weakThis.lock();
      if (!sharedThis)
        return false;
      if (sharedThis->m_editMode)
        return true;
      auto sharedModel = sharedThis->m_viewModel->model.lock();
      if (!sharedModel)
        return false;

      auto        listenersMap = sharedModel->getEventListeners(targetKey);
      std::string type = uiEventTypeToString(eventType);
      if (listenersMap.find(type) != listenersMap.end()) // hasUserListener
        return true;

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
        return true; // stop hit test

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
  const auto [viewWidth, viewHeight] = viewSize();
  if ((viewWidth >= pageWidth) && (viewHeight >= pageHeight))
  {
    return false;
  }

  // vertical scoll
  const auto oldOffset = m_view->contentOffset();
  auto       newOffset = oldOffset;

  newOffset.x += x;
  newOffset.x = std::max(viewWidth - pageWidth, newOffset.x);
  newOffset.x = std::min(newOffset.x, 0.0);

  newOffset.y += y;
  newOffset.y = std::max(viewHeight - pageHeight, newOffset.y);
  newOffset.y = std::min(newOffset.y, 0.0);

  if (doublesNearlyEqual(newOffset.x, oldOffset.x) && doublesNearlyEqual(newOffset.y, oldOffset.y))
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
  m_view->show(m_viewModel, false, &requiredFonts);
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

bool Presenter::presentFrame(
  const std::size_t        index,
  const app::FrameOptions& opts,
  app::AnimationCompletion completion)
{
  if (m_view)
  {
    const auto success = m_view->presentFrame(index, opts, completion);
    if (success)
      m_lastPresentFrameOptions = opts;
    return success;
  }
  return false;
}

bool Presenter::dismissFrame(app::AnimationCompletion completion)
{
  if (!m_view)
    return false;

  return m_view->dismissFrame(m_lastPresentFrameOptions, completion);
}

void Presenter::initHistory()
{
  return m_view->initHistory();
}

bool Presenter::popFrame(const app::PopOptions& opts, app::AnimationCompletion completion)
{
  ASSERT(m_view);
  return m_view->popFrame(opts, m_lastPushFrameAnimationOptions, completion);
}

std::shared_ptr<StateTree> Presenter::savedState(const std::string& instanceDescendantId)
{
  ASSERT(m_view);
  return m_view->savedState(instanceDescendantId);
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

bool Presenter::setCurrentFrameIndex(const std::size_t index, const bool updateHistory)
{
  const auto success = m_view->setCurrentFrameIndex(index, updateHistory, {}, {});
  if (success && updateHistory)
    m_lastPushFrameAnimationOptions = {};

  return success;
}

bool Presenter::pushFrame(
  const std::size_t              index,
  const bool                     updateHistory,
  const app::FrameOptions&       option,
  const app::AnimationCompletion completion)
{
  const auto success = m_view->pushFrame(index, updateHistory, option.animation, completion);
  if (success && updateHistory)
    m_lastPushFrameAnimationOptions = option.animation;

  return success;
}

bool Presenter::setInstanceState(
  const LayoutNode*             oldNode,
  const LayoutNode*             newNode,
  const app::UIAnimationOption& options,
  app::AnimationCompletion      completion)
{
  return m_view->setInstanceState(
    oldNode,
    newNode,
    options,
    [v = m_view, newNode, completion](bool finished)
    {
      v->updateState(newNode);

      if (completion)
        completion(finished);
    });
}

bool Presenter::presentInstanceState(
  const std::shared_ptr<StateTree>& oldState,
  const LayoutNode*                 oldNode,
  const LayoutNode*                 newNode,
  const app::UIAnimationOption&     options,
  app::AnimationCompletion          completion)
{
  m_lastPresentStateAnimationOptions = options;
  bool success = m_view->setInstanceState(oldNode, newNode, options, completion);
  m_view->saveState(oldState);
  return success;
}

std::unique_ptr<LayoutContext> Presenter::layoutContext()
{
  return m_view->layoutContext();
}

void Presenter::setBackgroundColor(uint32_t color)
{
  m_view->setBackgroundColor(color);
}

bool Presenter::setElementFillEnabled(
  const std::string&            id,
  std::size_t                   index,
  bool                          enabled,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementFillEnabled(id, index, enabled, animation);
}
bool Presenter::setElementFillColor(
  const std::string&            id,
  std::size_t                   index,
  float                         a,
  float                         r,
  float                         g,
  float                         b,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementFillColor(id, index, a, r, g, b, animation);
}

bool Presenter::setElementFillOpacity(
  const std::string&            id,
  std::size_t                   index,
  float                         opacity,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementFillOpacity(id, index, opacity, animation);
}
bool Presenter::setElementFillBlendMode(
  const std::string&            id,
  std::size_t                   index,
  int                           mode,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementFillBlendMode(id, index, mode, animation);
}
bool Presenter::setElementFillRotation(
  const std::string&            id,
  std::size_t                   index,
  float                         degree,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementFillRotation(id, index, degree, animation);
}

bool Presenter::setElementOpacity(
  const std::string&            id,
  float                         opacity,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementOpacity(id, opacity, animation);
}
bool Presenter::setElementVisible(
  const std::string&            id,
  bool                          visible,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementVisible(id, visible, animation);
}
bool Presenter::setElementMatrix(
  const std::string&            id,
  float                         a,
  float                         b,
  float                         c,
  float                         d,
  float                         tx,
  float                         ty,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementMatrix(id, a, b, c, d, tx, ty, animation);
}
bool Presenter::setElementSize(
  const std::string&            id,
  float                         width,
  float                         height,
  const app::UIAnimationOption& animation)
{
  return m_view->setElementSize(id, width, height, animation);
}

int Presenter::updateElement(
  const std::vector<app::UpdateElementItem>& items,
  const app::UIAnimationOption&              option)
{
  return m_view->updateElement(items, option);
}

} // namespace VGG