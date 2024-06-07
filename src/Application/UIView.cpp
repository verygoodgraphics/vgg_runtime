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
#include <nlohmann/json.hpp>

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

constexpr auto K_EMPTY_STRING = "";

#undef DEBUG
#define DEBUG(msg, ...)

#define VERBOSE DEBUG
#undef VERBOSE
#define VERBOSE(msg, ...)

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

} // namespace

namespace VGG
{

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

      // according to js event order
      EUIEventType types[] = { EUIEventType::MOUSEMOVE,
                               EUIEventType::MOUSEOVER,
                               EUIEventType::MOUSEENTER,
                               EUIEventType::MOUSEOUT,
                               EUIEventType::MOUSELEAVE };
      for (auto type : types)
      {
        handleMouseEvent(
          0,
          evt.motion.windowX,
          evt.motion.windowY,
          evt.motion.xrel,
          evt.motion.yrel,
          type);
      }
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

VGG::Layout::Point UIView::converPointFromWindowAndScale(Layout::Point point)
{
  const auto o = offset();
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
  EUIEventType type)
{
  auto page = currentPage();
  if (!page)
  {
    return false;
  }

  bool success = dispatchMouseEventOnPage(page, jsButtonIndex, x, y, motionX, motionY, type, true);

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
      success |= dispatchMouseEventOnPage(fromPage, jsButtonIndex, x, y, motionX, motionY, type);
    }
  }

  for (auto& stateTree : m_stateTrees)
    success |= dispatchMouseEventOnPage(stateTree, jsButtonIndex, x, y, motionX, motionY, type);

  return success;
}

bool UIView::dispatchMouseEventOnPage(
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

  Layout::Point pointToPage =
    converPointFromWindowAndScale({ TO_VGG_LAYOUT_SCALAR(x), TO_VGG_LAYOUT_SCALAR(y) });
  Layout::Point pointToDocument{ pointToPage.x + page->asPage()->frame().origin.x,
                                 pointToPage.y + page->asPage()->frame().origin.y };

  VERBOSE(
    "UIView::dispatchMouseEventOnPage, page: mouse: %d, %d; point to page: %f, %f; point to "
    "document: %f, %f",
    x,
    y,
    pointToPage.x,
    pointToPage.y,
    pointToDocument.x,
    pointToDocument.y);

  auto target = page->hitTest(
    pointToDocument,
    [&queryHasEventListener = m_hasEventListener, type](const std::string& targetKey)
    { return queryHasEventListener(targetKey, type); });
  std::shared_ptr<VGG::LayoutNode> hitNodeInTarget;
  if (target.first)
  {
    hitNodeInTarget = target.first->hitTest(pointToDocument, nullptr).first;
  }

  if (updateCursor && (type == EUIEventType::MOUSEMOVE))
    page->hitTest(
      pointToDocument,
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
          pointToDocument,
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
          return true;
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
        return true;
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
      return true;
    }
    break;

    case EUIEventType::MOUSELEAVE:
    {
      handleMouseLeave(eventContext, target, jsButtonIndex, x, y, motionX, motionY);
      return true;
    }
    break;

    case EUIEventType::CLICK:
    case EUIEventType::AUXCLICK:
    case EUIEventType::CONTEXTMENU:
    {
      if (target.first && m_possibleClickTargetNode != target)
      {
        return false;
      }
    }
    break;

    default:
      break;
  }

  fireMouseEvent(target, type, jsButtonIndex, pointToPage.x, pointToPage.y, motionX, motionY);
  return true;
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

void UIView::handleMouseLeave(
  EventContext& eventContext,
  TargetNode    target,
  int           jsButtonIndex,
  int           x,
  int           y,
  int           motionX,
  int           motionY)
{
  TargetNode fireTarget;

  if (target.first)
  {
    if (eventContext.mouseLeaveTargetNode.first)
    {
      if (target == eventContext.mouseLeaveTargetNode)
      {
        return;
      }
      else
      {
        fireTarget = eventContext.mouseLeaveTargetNode;
      }
    }

    DEBUG(
      "mouse leave target node: %s, %s",
      target.first->name().c_str(),
      target.first->id().c_str());
    eventContext.mouseLeaveTargetNode = target;
  }
  else
  {
    if (eventContext.mouseLeaveTargetNode.first)
    {
      fireTarget = eventContext.mouseLeaveTargetNode;
      DEBUG("mouse leave target set to null");
      eventContext.mouseLeaveTargetNode = {};
    }
  }

  if (fireTarget.first)
  {
    DEBUG(
      "mouse leave node: %s, %s",
      fireTarget.first->name().c_str(),
      fireTarget.first->id().c_str());
    fireMouseEvent(fireTarget, EUIEventType::MOUSELEAVE, jsButtonIndex, x, y, motionX, motionY);
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
  {
    return false;
  }

  Layout::Point pointToPage =
    converPointFromWindowAndScale({ TO_VGG_LAYOUT_SCALAR(x), TO_VGG_LAYOUT_SCALAR(y) });
  Layout::Point pointToDocument{ pointToPage.x + page->frame().origin.x,
                                 pointToPage.y + page->frame().origin.y };

  auto [target, key] = page->hitTest(
    pointToDocument,
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
  if (auto& n = currentContext.mouseLeaveTargetNode.first; n && (n.get() != instanceNode))
  {
    DEBUG("UIView::saveState, move mouse leave node to old state tree: %s", n->id().c_str());
    oldContext.mouseLeaveTargetNode = currentContext.mouseLeaveTargetNode;
    currentContext.mouseLeaveTargetNode = {};
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
  onEvent(m_lastMouseMove, nullptr);
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

layer::Ref<layer::SceneNode> UIView::getSceneNode()
{
  return m_impl->sceneNode();
}

void UIView::frame()
{
  m_impl->frame();
  setDirty(false);
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
    return true;

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

bool UIView::updateNodeFillColor(
  const std::string& id,
  const std::size_t  fillIndex,
  const double       r,
  const double       g,
  const double       b,
  const double       a)
{
  const auto success = m_impl->updateNodeFillColor(id, fillIndex, r, g, b, a);
  if (success)
    setDirty(true);

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

} // namespace VGG
