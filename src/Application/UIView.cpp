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
#include "UIView.hpp"
#include <optional>
#include "Domain/Layout/LayoutNode.hpp"
#include "Domain/Model/Element.hpp"
#include "Event/Event.hpp"
#include "Event/EventAPI.hpp"
#include "Event/Keycode.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/SceneBuilder.hpp"
#include "UIAnimation.hpp"
#include "UIOptions.hpp"
#include "UIViewImpl.hpp"
#include "Utility/Log.hpp"
#include "ViewModel.hpp"
#include <core/SkColor.h>
#include <glm/detail/qualifier.hpp>

#undef DEBUG
#define DEBUG(msg, ...)

#define VERBOSE DEBUG
#undef VERBOSE
#define VERBOSE(msg, ...)

namespace VGG
{

constexpr auto K_EMPTY_STRING = "";

namespace
{
bool operator==(
  const std::pair<std::shared_ptr<LayoutNode>, std::string>& lhs,
  const std::pair<std::shared_ptr<LayoutNode>, std::string>& rhs)
{
  if (lhs.first && rhs.first)
  {
    return lhs.first->id() == rhs.first->id();
  }
  return false;
}

Layout::Point pointToDocument(int x, int y, Layout::Point pointToPage, const LayoutNode& page)
{
  Layout::Point p{ pointToPage.x + page.asPage()->frame().origin.x,
                   pointToPage.y + page.asPage()->frame().origin.y };

  VERBOSE(
    "page: mouse: %d, %d; point to page: %f, %f; point to document: %f, %f",
    x,
    y,
    pointToPage.x,
    pointToPage.y,
    p.x,
    p.y);

  return p;
}

} // namespace

UIView::UIView()
  : m_impl{ new internal::UIViewImpl(this) }
{
}

UIView::~UIView() = default;

bool UIView::onEvent(UEvent evt, void* userData)
{
  VERBOSE("UIView::onEvent, 0x%x", evt.type);

  if (m_isZoomerEnabled && m_impl->onEvent(evt, userData))
  {
    // zoom or pan event
    setDirty(true);
    return true;
  }

  if (!m_eventListener)
  {
    return false;
  }

  // todo, capturing
  // todo, bubbling
  switch (evt.type)
  {
    case VGG_MOUSEBUTTONDOWN:
    {
      auto jsButtonIndex{ evt.button.button - 1 };
      return handleMouseEvent(
        jsButtonIndex,
        evt.button.windowX,
        evt.button.windowY,
        0,
        0,
        EUIEventType::MOUSEDOWN);
    }
    break;

    case VGG_MOUSEMOTION:
    {
      m_lastMouseMove = evt;
      onMouseMove(evt);
      return true;
    }
    break;

    case VGG_MOUSEBUTTONUP:
    {
      auto jsButtonIndex{ evt.button.button - 1 };

      handleMouseEvent(
        jsButtonIndex,
        evt.button.windowX,
        evt.button.windowY,
        0,
        0,
        EUIEventType::MOUSEUP);

      if (jsButtonIndex == 0)
      {
        handleMouseEvent(
          jsButtonIndex,
          evt.button.windowX,
          evt.button.windowY,
          0,
          0,
          EUIEventType::CLICK);
      }
      else
      {
        handleMouseEvent(
          jsButtonIndex,
          evt.button.windowX,
          evt.button.windowY,
          0,
          0,
          EUIEventType::AUXCLICK);

        if (jsButtonIndex == 2)
        {
          handleMouseEvent(
            jsButtonIndex,
            evt.button.windowX,
            evt.button.windowY,
            0,
            0,
            EUIEventType::CONTEXTMENU);
        }
      }

      m_possibleClickTargetNode = {};
    }
    break;

    case VGG_MOUSEWHEEL:
    {
    }
    break;

    case VGG_KEYDOWN:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
      m_eventListener(
        UIEventPtr(new KeyboardEvent(
          EUIEventType::KEYDOWN,
          {},
          {},
          {},
          evt.key.keysym.sym,
          evt.key.repeat,
          alt,
          ctrl,
          meta,
          shift)),
        {});
    }
    break;

    case VGG_KEYUP:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
      m_eventListener(
        UIEventPtr(new KeyboardEvent(
          EUIEventType::KEYUP,
          {},
          {},
          {},
          evt.key.keysym.sym,
          evt.key.repeat,
          alt,
          ctrl,
          meta,
          shift)),
        {});
    }
    break;

    case VGG_TOUCHDOWN:
    {
      handleTouchEvent(evt.touch.windowX, evt.touch.windowY, 0, 0, EUIEventType::TOUCHSTART);
    }
    break;

    case VGG_TOUCHMOTION:
    {
      handleTouchEvent(
        evt.touch.windowX,
        evt.touch.windowY,
        evt.touch.xrel,
        evt.touch.yrel,
        EUIEventType::TOUCHMOVE);
    }
    break;

    case VGG_TOUCHUP:
    {
      handleTouchEvent(evt.touch.windowX, evt.touch.windowY, 0, 0, EUIEventType::TOUCHEND);
    }
    break;

      // todo, more event

    default:
      break;
  }

  return true;
}

std::tuple<bool, bool, bool, bool> UIView::getKeyModifier(int keyMod)
{
  bool lAlt = keyMod & VGG_KMOD_LALT;
  bool lCtrl = keyMod & VGG_KMOD_LCTRL;
  bool lMeta = keyMod & VGG_KMOD_LGUI;
  bool lShift = keyMod & VGG_KMOD_LSHIFT;

  bool rAlt = keyMod & VGG_KMOD_RALT;
  bool rCtrl = keyMod & VGG_KMOD_RCTRL;
  bool rMeta = keyMod & VGG_KMOD_RGUI;
  bool rShift = keyMod & VGG_KMOD_RSHIFT;

  return { lAlt || rAlt, lCtrl || rCtrl, lMeta || rMeta, lShift || rShift };
}

void UIView::becomeEditorWithSidebar(
  ScalarType top,
  ScalarType right,
  ScalarType bottom,
  ScalarType left)
{
  m_isEditor = true;

  m_top = top;
  m_right = right;
  m_bottom = bottom;
  m_left = left;

  // todo, editor layout
}

void UIView::layoutSubviews()
{
  if (m_isEditor)
  {
    for (auto& subview : m_subviews)
    {
      subview->m_frame = { { TO_VGG_LAYOUT_SCALAR(m_left), TO_VGG_LAYOUT_SCALAR(m_top) },
                           {
                             m_frame.size.width - m_left - m_right,
                             m_frame.size.height - m_top - m_bottom,
                           } };
    }
  }
}

Layout::Point UIView::converPointFromWindowAndScale(int x, int y)
{
  Layout::Point point{ TO_VGG_LAYOUT_SCALAR(x), TO_VGG_LAYOUT_SCALAR(y) };
  const auto    o = offset();
  point.x -= o.x;
  point.y -= o.y;

  auto scaleFactor = scale();
  point.x /= scaleFactor;
  point.y /= scaleFactor;

  return point;
}

int UIView::currentPageIndex()
{
  return m_impl->page();
}

std::shared_ptr<LayoutNode> UIView::currentPage()
{
  auto document = m_document.lock();
  if (document)
  {
    return document->children()[m_impl->page()];
  }

  return nullptr;
}

std::shared_ptr<LayoutNode> UIView::pageById(const std::string& id)
{
  auto document = m_document.lock();
  if (document)
  {
    for (const auto& page : document->children())
    {
      if (page->id() == id)
      {
        return page;
      }
    }
  }

  return nullptr;
}

const std::string UIView::pageIdByIndex(std::size_t index)
{
  if (const auto& doc = m_document.lock())
    if (index < doc->children().size())
      return doc->children()[index]->id();

  return {};
}

bool UIView::handleMouseEvent(
  int          jsButtonIndex,
  int          x,
  int          y,
  int          motionX,
  int          motionY,
  EUIEventType type,
  bool         forHover)
{
  auto page = currentPage();
  if (!page)
    return false;

  auto r = dispatchMouseEventOnPage(page, jsButtonIndex, x, y, motionX, motionY, type, true);
  if (forHover) // dipatch on real page only, skip presenting page or state tree
    return r.handled;
  if (r.handled && r.hasTarget)
  {
    DEBUG(
      "mouse event %s is handled on page: %s, %s",
      uiEventTypeToString(type),
      page->name().c_str(),
      page->id().c_str());
    return true;
  }

  const auto& currentPageId = page->id();
  if (m_presentingPages.contains(currentPageId))
  {
    const auto& fromPageId = m_presentingPages[currentPageId];
    if (const auto& fromPage = pageById(fromPageId))
    {
      DEBUG(
        "dispatch mouse event on from page: %s, %s",
        fromPage->name().c_str(),
        fromPage->id().c_str());
      auto r2 = dispatchMouseEventOnPage(fromPage, jsButtonIndex, x, y, motionX, motionY, type);
      if (r2.handled && r2.hasTarget)
      {
        DEBUG(
          "mouse event %s is handled on page: %s, %s",
          uiEventTypeToString(type),
          fromPage->name().c_str(),
          fromPage->id().c_str());
        return true;
      }

      r.handled |= r2.handled;
    }
  }

  for (auto it = m_stateTrees.rbegin(); it != m_stateTrees.rend(); ++it)
  {
    auto r3 = dispatchMouseEventOnPage(*it, jsButtonIndex, x, y, motionX, motionY, type);
    switch (type)
    {
      case EUIEventType::MOUSEENTER:
      case EUIEventType::MOUSELEAVE:
        if (r3.handled && r3.hasTarget)
        {
          DEBUG(
            "mouse event %s is handled in state tree: %s, %s",
            uiEventTypeToString(type),
            (*it)->asPage()->name().c_str(),
            (*it)->asPage()->id().c_str());
          return true;
        }

      default:
        break;
    }

    r.handled |= r3.handled;
  }

  return r.handled;
}

UIView::EventHandleResult UIView::dispatchMouseEventOnPage(
  std::shared_ptr<LayoutNode> page,
  int                         jsButtonIndex,
  int                         x,
  int                         y,
  int                         motionX,
  int                         motionY,
  EUIEventType                type,
  bool                        updateCursor)
{
  VERBOSE(
    "UIView::dispatchMouseEventOnPage, page: %s, %s",
    page->name().c_str(),
    page->id().c_str());

  auto& eventContext = m_presentedTreeContext[page->id()];

  const Layout::Point pointToPage = converPointFromWindowAndScale(x, y);
  const auto          p = pointToDocument(x, y, pointToPage, *page);

  auto target = page->hitTest(
    p,
    [&queryHasEventListener = m_hasEventListener, type](const std::string& targetKey)
    { return queryHasEventListener(targetKey, type); });
  std::shared_ptr<VGG::LayoutNode> hitNodeInTarget;
  if (target.first)
  {
    hitNodeInTarget = target.first->hitTest(p, nullptr).first;
  }

  if (updateCursor && (type == EUIEventType::MOUSEMOVE))
    page->hitTest(
      p,
      [&updateCursorEventListener = m_updateCursorEventListener, type](const std::string& targetKey)
      { return updateCursorEventListener(targetKey, type); });

  switch (type)
  {
    case EUIEventType::MOUSEDOWN:
    {
      EUIEventType types[] = { EUIEventType::CLICK,
                               EUIEventType::AUXCLICK,
                               EUIEventType::CONTEXTMENU };
      for (auto clickType : types)
      {
        auto clickTarget = page->hitTest(
          p,
          [&queryHasEventListener = m_hasEventListener, clickType](const std::string& targetKey)
          { return queryHasEventListener(targetKey, clickType); });
        if (clickTarget.first)
        {
          m_possibleClickTargetNode = clickTarget;
          DEBUG(
            "mousedown, m_possibleClickTargetNode: %s, %s",
            m_possibleClickTargetNode.first->name().c_str(),
            m_possibleClickTargetNode.first->id().c_str());
          break;
        }
      }
    }
    break;

    case EUIEventType::MOUSEOVER:
    {
      if (target.first)
      {
        if (
          target == eventContext.mouseOverTargetNode &&
          eventContext.mouseOverNode == hitNodeInTarget)
        {
          return { true, true };
        }
        eventContext.mouseOverTargetNode = target;
        eventContext.mouseOverNode = hitNodeInTarget;
      }
      else
      {
        eventContext.mouseOverTargetNode = {};
        eventContext.mouseOverNode = nullptr;
      }
    }
    break;

    case EUIEventType::MOUSEENTER:
    {
      if (target.first && target == eventContext.mouseEnterTargetNode)
      {
        DEBUG(
          "mouse enter same node: %s, %s",
          target.first->name().c_str(),
          target.first->id().c_str());
        return { true, true };
      }
      if (auto node = target.first)
      {
        DEBUG("mouse enter node: %s, %s", node->name().c_str(), node->id().c_str());
      }
      eventContext.mouseEnterTargetNode = target;
    }
    break;

    case EUIEventType::MOUSEOUT:
    {
      handleMouseOut(eventContext, target, hitNodeInTarget, jsButtonIndex, x, y, motionX, motionY);
      return { true, !!target.first };
    }
    break;

    case EUIEventType::MOUSELEAVE:
    {
      auto handled =
        handleMouseLeave(eventContext, target, p, jsButtonIndex, pointToPage, motionX, motionY);
      return { true, handled };
    }
    break;

    case EUIEventType::CLICK:
    case EUIEventType::AUXCLICK:
    case EUIEventType::CONTEXTMENU:
    {
      if (target.first && m_possibleClickTargetNode != target)
      {
        return { false, true };
      }
    }
    break;

    default:
      break;
  }

  fireMouseEvent(target, type, jsButtonIndex, pointToPage.x, pointToPage.y, motionX, motionY);
  return { true, !!target.first };
}

void UIView::handleMouseOut(
  EventContext&                    eventContext,
  TargetNode                       target,
  std::shared_ptr<VGG::LayoutNode> hitNodeInTarget,
  int                              jsButtonIndex,
  int                              x,
  int                              y,
  int                              motionX,
  int                              motionY)
{
  TargetNode fireTarget;

  if (target.first)
  {
    if (eventContext.mouseOutTargetNode.first)
    {
      if (target == eventContext.mouseOutTargetNode)
      {
        if (eventContext.mouseOutNode != hitNodeInTarget)
        {
          fireTarget = eventContext.mouseOutTargetNode;
          eventContext.mouseOutNode = hitNodeInTarget;
        }
      }
      else
      {
        fireTarget = eventContext.mouseOutTargetNode;
        eventContext.mouseOutTargetNode = target;
        eventContext.mouseOutNode = hitNodeInTarget;
      }
    }
    else
    {
      DEBUG(
        "m_mouseOutTargetNode: %s, %s",
        fireTarget.first->name().c_str(),
        fireTarget.first->id().c_str());
      eventContext.mouseOutTargetNode = target;
      eventContext.mouseOutNode = hitNodeInTarget;
      return;
    }
  }
  else
  {
    if (eventContext.mouseOutTargetNode.first)
    {
      fireTarget = eventContext.mouseOutTargetNode;
      DEBUG("mouse out of the m_mouseOutTargetNode, set it to nullptr");
      eventContext.mouseOutTargetNode = {};
      eventContext.mouseOutNode = nullptr;
    }
  }

  if (fireTarget.first)
  {
    fireMouseEvent(fireTarget, EUIEventType::MOUSEOUT, jsButtonIndex, x, y, motionX, motionY);
  }
}

bool UIView::handleMouseLeave(
  EventContext& eventContext,
  TargetNode    targetNodeAndKey,
  Layout::Point pointToDocument,
  int           button, // jsButtonIndex
  Layout::Point p,      // point to page
  int           motionX,
  int           motionY)
{
  auto& targets = eventContext.mouseLeaveTargetNodes;
  if (targetNodeAndKey.first)
  {
    if (auto it = std::find(targets.begin(), targets.end(), targetNodeAndKey); it != targets.end())
    {
      for (auto leaveIt = targets.rbegin();
           (leaveIt != targets.rend()) && (*leaveIt != targetNodeAndKey);
           leaveIt = targets.rbegin())
      {
        DEBUG(
          "mouse leave node: %s, %s",
          leaveIt->first->name().c_str(),
          leaveIt->first->id().c_str());
        fireMouseEvent(*leaveIt, EUIEventType::MOUSELEAVE, button, p.x, p.y, motionX, motionY);
        targets.pop_back();
      }
    }
    else
    {
      std::erase_if(
        targets,
        [this, pointToDocument, button, p, motionX, motionY](TargetNode& t)
        {
          if (!t.first->pointInside(pointToDocument))
          {
            DEBUG("mouse leave node: %s, %s", t.first->name().c_str(), t.first->id().c_str());
            fireMouseEvent(t, EUIEventType::MOUSELEAVE, button, p.x, p.y, motionX, motionY);
            return true;
          }
          else
            return false;
        });

      DEBUG(
        "mouse leave target node: %s, %s",
        targetNodeAndKey.first->name().c_str(),
        targetNodeAndKey.first->id().c_str());
      targets.push_back(targetNodeAndKey);
    }

    return true; // handled
  }
  else
  {
    if (targets.empty())
      return false;

    for (auto it = targets.rbegin(); it != targets.rend(); ++it)
    {
      DEBUG("mouse leave node: %s, %s", it->first->name().c_str(), it->first->id().c_str());
      fireMouseEvent(*it, EUIEventType::MOUSELEAVE, button, p.x, p.y, motionX, motionY);
    }
    targets.clear();
    return true;
  }
}

void UIView::fireMouseEvent(
  TargetNode        target,
  VGG::EUIEventType type,
  int               jsButtonIndex,
  int               x,
  int               y,
  int               motionX,
  int               motionY)
{
  auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
  auto [node, key] = target;
  m_eventListener(
    UIEventPtr(new MouseEvent(
      type,
      node ? node->id() : K_EMPTY_STRING,
      node ? node->name() : K_EMPTY_STRING,
      key,
      jsButtonIndex,
      x,
      y,
      motionX,
      motionY,
      alt,
      ctrl,
      meta,
      shift)),
    node);
}

void UIView::show(
  std::shared_ptr<ViewModel>&                viewModel,
  bool                                       force,
  std::unordered_map<std::string, FontInfo>* requiredFonts)
{
  ASSERT(viewModel);
  if (m_skipUntilNextLoop && !force)
    return;

  std::vector<layer::StructFrameObject> frames;
  for (auto& f : *viewModel->designDoc())
    if (
      (f->type() == VGG::Domain::Element::EType::FRAME) &&
      static_pointer_cast<Domain::FrameElement>(f)->shouldDisplay())
      frames.emplace_back(layer::StructFrameObject(f.get()));

  auto result =
    layer::SceneBuilder::builder()
      .setFontNameVisitor(
        [requiredFonts](const std::string& familyName, const std::string& subfamilyName)
        {
          if (requiredFonts)
            (*requiredFonts)[familyName + subfamilyName] = FontInfo{ familyName, subfamilyName };
        })
      .build<layer::StructModelFrame>(std::move(frames));
  if (result.root)
    show(viewModel, std::move(*result.root), force);
  else
    WARN("#UIView::show, built scene is empty");
}

void UIView::show(
  std::shared_ptr<ViewModel>&  viewModel,
  std::vector<layer::FramePtr> frames,
  bool                         force)
{
  ASSERT(viewModel);
  if (m_skipUntilNextLoop && !force)
  {
    return;
  }
  m_skipUntilNextLoop = true;

  m_impl->show(viewModel, frames);
  m_impl->setPageIndex(m_impl->page());

  m_document = viewModel->layoutTree();

  setDirty(true);
}

void UIView::setOffsetAndScale(float xOffset, float yOffset, float scale)
{
  m_impl->setOffsetAndScale(xOffset, yOffset, scale);
}

void UIView::resetOffsetAndScale()
{
  m_impl->resetOffsetAndScale();
}

void UIView::enableZoomer(bool enabled)
{
  m_isZoomerEnabled = enabled;
}

bool UIView::handleTouchEvent(int x, int y, int motionX, int motionY, EUIEventType type)
{
  auto page = currentPage();
  if (!page)
    return false;

  auto [target, key] = page->hitTest(
    pointToDocument(x, y, converPointFromWindowAndScale(x, y), *page),
    [&queryHasEventListener = m_hasEventListener, type](const std::string& targetKey)
    { return queryHasEventListener(targetKey, type); });

  m_eventListener(
    UIEventPtr(new TouchEvent(
      type,
      target ? target->id() : K_EMPTY_STRING,
      target ? target->name() : K_EMPTY_STRING,
      key)),
    target);

  return true;
}

bool UIView::presentFrame(
  const std::size_t        index,
  const app::FrameOptions& opts,
  app::AnimationCompletion completion)
{
  const auto& from = currentPage()->id();
  if (setCurrentFrameIndex(index, false, opts.animation, completion))
  {
    const auto& to = pageIdByIndex(index);
    m_presentedPages[from] = to;
    m_presentingPages[to] = from;
    return true;
  }

  return false;
}

bool UIView::dismissFrame(const app::FrameOptions& opts, app::AnimationCompletion completion)
{
  const auto& to = currentPage()->id();
  if (!m_presentingPages.contains(to))
    return false;

  const auto copiedFrom = m_presentingPages[to];
  m_presentingPages.erase(to);
  m_presentedPages.erase(copiedFrom);
  m_presentedTreeContext.erase(to);

  const auto& document = m_document.lock();
  if (document)
    for (std::size_t index = 0; index < document->children().size(); ++index)
      if (document->children()[index]->id() == copiedFrom)
        return setCurrentFrameIndex(index, false, opts.animation, completion);

  return false;
}

bool UIView::popFrame(
  const app::PopOptions&        popOpts,
  const app::UIAnimationOption& animationOpts,
  app::AnimationCompletion      completion)
{
  if (m_history.size() <= 1)
  {
    return false;
  }

  auto displayedPage = currentPage();

  m_history.pop();
  auto backToPageId = m_history.top();

  if (auto document = m_document.lock())
  {
    for (std::size_t index = 0; index < document->children().size(); ++index)
    {
      if (document->children()[index]->id() == backToPageId)
      {
        // clear presented page relations
        auto to = displayedPage->id();
        while (!to.empty())
        {
          m_presentedTreeContext.erase(to);

          if (!m_presentingPages.contains(to))
          {
            break;
          }

          auto from = m_presentingPages[to];
          m_presentingPages.erase(to);
          m_presentedPages.erase(from);

          to = from;
        }

        return setCurrentFrameIndex(index, false, animationOpts, completion);
      }
    }
  }

  return false;
}

void UIView::saveState(const std::shared_ptr<StateTree>& stateTree)
{
  ASSERT(stateTree);
  DEBUG("UIView::saveState, save state tree: %s", stateTree->id().c_str());
  auto it = std::find_if(
    m_stateTrees.begin(),
    m_stateTrees.end(),
    [&stateTree](auto& s) { return s->id() == stateTree->id(); });
  if (it != m_stateTrees.end())
    *it = stateTree;
  else
    m_stateTrees.push_back(stateTree);

  EventContext& currentContext = m_presentedTreeContext[currentPage()->id()];
  EventContext& oldContext = m_presentedTreeContext[stateTree->id()]; // make context

  LayoutNode* instanceNode = stateTree->srcNode().get();
  if (auto& n = currentContext.mouseOverTargetNode.first; n && (n.get() != instanceNode))
  {
    DEBUG("UIView::saveState, move mouse over node to old state tree: %s", n->id().c_str());
    oldContext.mouseOverTargetNode = currentContext.mouseOverTargetNode;
    oldContext.mouseOverNode = currentContext.mouseOverNode;
    currentContext.mouseOverTargetNode = {};
    currentContext.mouseOverNode.reset();
  }
  if (auto& n = currentContext.mouseEnterTargetNode.first; n && (n.get() != instanceNode))
  {
    DEBUG("UIView::saveState, move mouse enter node to old state tree: %s", n->id().c_str());
    oldContext.mouseEnterTargetNode = currentContext.mouseEnterTargetNode;
    currentContext.mouseEnterTargetNode = {};
  }
  if (auto& n = currentContext.mouseOutTargetNode.first; n && (n.get() != instanceNode))
  {
    DEBUG("UIView::saveState, move mouse out node to old state tree: %s", n->id().c_str());
    oldContext.mouseOutTargetNode = currentContext.mouseOutTargetNode;
    oldContext.mouseOutNode = currentContext.mouseOutNode;
    currentContext.mouseOutTargetNode = {};
    currentContext.mouseOutNode.reset();
  }

  std::vector<TargetNode> targets = std::move(currentContext.mouseLeaveTargetNodes);
  for (auto& t : targets)
  {
    if (t.first->isAncestorOf(instanceNode))
      currentContext.mouseLeaveTargetNodes.push_back(t);
    else
    {
      DEBUG(
        "UIView::saveState, move mouse leave node to old state tree: %s",
        t.first->id().c_str());
      oldContext.mouseLeaveTargetNodes.push_back(t);
    }
  }
}

std::shared_ptr<StateTree> UIView::savedState(const std::string& instanceDescendantId)
{
  for (auto& s : m_stateTrees)
    if (s->findDescendantNodeById(instanceDescendantId))
      return s;
  return nullptr;
}

void UIView::restoreState(const std::string& instanceId)
{
  auto it = std::find_if(
    m_stateTrees.begin(),
    m_stateTrees.end(),
    [&instanceId](auto& s) { return s->id() == instanceId; });
  if (it != m_stateTrees.end())
  {
    DEBUG("UIView::restoreState, restore state tree: %s", (*it)->id().c_str());
    m_presentedTreeContext.erase((*it)->id());
    m_stateTrees.erase(it);
  }
}

void UIView::triggerMouseEnter()
{
  DEBUG("UIView::triggerMouseEnter");
  if (m_lastMouseMove.has_value())
    onMouseMove(*m_lastMouseMove, true);
}

void UIView::setLayer(app::AppRender* layer)
{
  m_impl->setLayer(layer);
}

float UIView::scale() const
{
  return m_impl->scale();
}
glm::vec2 UIView::offset() const
{
  return m_impl->offset();
}
void UIView::setOffset(float dx, float dy)
{
  m_impl->setOffset({ dx, dy });
  setDirty(true);
}
void UIView::translate(float dx, float dy)
{
  m_impl->translate(dx, dy);
  setDirty(true);
}

void UIView::setDrawBackground(bool drawBackground)
{
  m_drawGrayBackground = drawBackground;

  auto color = SK_ColorWHITE;
  if (m_drawGrayBackground)
  {
    if (auto page = currentPage())
    {
      color = page->backgroundColor();
    }
  }

  m_impl->setBackgroundColor(color);
}

void UIView::setBackgroundColor(uint32_t color)
{
  m_impl->setBackgroundColor(color);
}

layer::Ref<layer::SceneNode> UIView::getSceneNode()
{
  return m_impl->sceneNode();
}

void UIView::frame()
{
  setDirty(m_impl->deleteFinishedAnimation());
}

bool UIView::isDirty()
{
  if (m_isDirty)
  {
    return true;
  }
  else if (m_impl->isAnimating())
  {
    m_paintOnceMoreAfterAnimation = true;
    return true;
  }
  else if (m_paintOnceMoreAfterAnimation)
  {
    m_paintOnceMoreAfterAnimation = false;
    return true;
  }
  else
  {
    return false;
  }
}

bool UIView::setCurrentFrameIndex(
  const std::size_t                   index,
  const bool                          updateHistory,
  const app::UIAnimationOption&       option,
  const VGG::app::AnimationCompletion completion)
{
  if (m_impl->page() == static_cast<int>(index))
    return false;

  const auto document = m_document.lock();
  if (!document)
    return false;

  if (index >= document->children().size())
    return false;

  const auto success = m_impl->setPageIndexAnimated(
    index,
    option,
    [updateHistory, completion, this](bool finished)
    {
      if (finished && updateHistory)
        m_history.push(currentPage()->id());
      if (completion)
        completion(finished);
    });

  if (success)
    setDirty(true);

  return success;
}

bool UIView::pushFrame(
  const std::size_t                   index,
  const bool                          updateHistory,
  const app::UIAnimationOption&       option,
  const VGG::app::AnimationCompletion completion)
{
  const auto success = setCurrentFrameIndex(index, updateHistory, option, completion);
  if (success)
  {
    m_presentedTreeContext.clear();
    m_presentedPages.clear();
    m_presentingPages.clear();
  }
  return success;
}

void UIView::initHistory()
{
  m_history.push(currentPage()->id());
}

bool UIView::setInstanceState(
  const LayoutNode*             oldNode,
  const LayoutNode*             newNode,
  const app::UIAnimationOption& options,
  app::AnimationCompletion      completion)
{
  ASSERT(oldNode);
  restoreState(oldNode->id());
  return m_impl->setInstanceState(oldNode, newNode, options, completion);
}
bool UIView::presentInstanceState(
  const LayoutNode*             oldNode,
  const LayoutNode*             newNode,
  const app::UIAnimationOption& options,
  app::AnimationCompletion      completion)
{
  return m_impl->setInstanceState(oldNode, newNode, options, completion);
}

std::unique_ptr<LayoutContext> UIView::layoutContext()
{
  return m_impl->layoutContext();
}

void UIView::onMouseMove(UEvent evt, bool forHover)
{
  // according to js event order
  EUIEventType types[] = { EUIEventType::MOUSEMOVE,
                           EUIEventType::MOUSEOVER,
                           EUIEventType::MOUSEENTER,
                           EUIEventType::MOUSEOUT,
                           EUIEventType::MOUSELEAVE };
  const auto&  m = evt.motion;
  for (auto type : types)
    handleMouseEvent(0, m.windowX, m.windowY, m.xrel, m.yrel, type, forHover);
}

void UIView::updateState(const LayoutNode* instanceNode)
{
  ASSERT(instanceNode);
  DEBUG("UIView::updateState, instance node : %s", instanceNode->id().c_str());

  EventContext& currentContext = m_presentedTreeContext[currentPage()->id()];
  if (auto& n = currentContext.mouseOverTargetNode.first; n && (n.get() == instanceNode))
  {
    DEBUG("UIView::updateState, clear mouse over node: %s", n->id().c_str());
    currentContext.mouseOverTargetNode = {};
    currentContext.mouseOverNode.reset();
  }
  if (auto& n = currentContext.mouseEnterTargetNode.first; n && (n.get() == instanceNode))
  {
    DEBUG("UIView::updateState, clear mouse enter node: %s", n->id().c_str());
    currentContext.mouseEnterTargetNode = {};
  }
  if (auto& n = currentContext.mouseOutTargetNode.first; n && (n.get() == instanceNode))
  {
    DEBUG("UIView::updateState, clear mouse out node: %s", n->id().c_str());
    currentContext.mouseOutTargetNode = {};
    currentContext.mouseOutNode.reset();
  }

  std::vector<TargetNode> targets = std::move(currentContext.mouseLeaveTargetNodes);
  for (auto& t : targets)
    if (t.first->isAncestorOf(instanceNode) && (t.first.get() != instanceNode))
      currentContext.mouseLeaveTargetNodes.push_back(t);
}

bool UIView::setElementFillEnabled(
  const std::string&            id,
  std::size_t                   index,
  bool                          enabled,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementFillEnabled(id, index, enabled, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementFillColor(
  const std::string&            id,
  std::size_t                   index,
  float                         a,
  float                         r,
  float                         g,
  float                         b,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementFillColor(id, index, a, r, g, b, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementFillOpacity(
  const std::string&            id,
  std::size_t                   index,
  float                         opacity,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementFillOpacity(id, index, opacity, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementFillBlendMode(
  const std::string&            id,
  std::size_t                   index,
  int                           mode,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementFillBlendMode(id, index, mode, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementFillRotation(
  const std::string&            id,
  std::size_t                   index,
  float                         degree,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementFillRotation(id, index, degree, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementOpacity(
  const std::string&            id,
  float                         opacity,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementOpacity(id, opacity, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementVisible(
  const std::string&            id,
  bool                          visible,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementVisible(id, visible, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementMatrix(
  const std::string&            id,
  float                         a,
  float                         b,
  float                         c,
  float                         d,
  float                         tx,
  float                         ty,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementMatrix(id, a, b, c, d, tx, ty, animation))
    setDirty(true);
  return false;
}

bool UIView::setElementSize(
  const std::string&            id,
  float                         width,
  float                         height,
  const app::UIAnimationOption& animation)
{
  if (m_impl->setElementSize(id, width, height, animation))
    setDirty(true);
  return false;
}

} // namespace VGG
