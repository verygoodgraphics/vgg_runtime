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

#include "Event/Event.hpp"
#include "Event/EventAPI.hpp"
#include "Event/Keycode.hpp"
#include "ViewModel.hpp"

#include "Domain/Layout/Node.hpp"
#include "Layer/Core/RasterCacheTile.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Layer/Zoomer.hpp"
#include "Utility/Log.hpp"

#include <include/core/SkCanvas.h>

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

constexpr auto K_EMPTY_STRING = "";

#undef DEBUG
#define DEBUG(msg, ...)

UIView::UIView()
  : AppScene(std::make_unique<VGG::layer::RasterCacheTile>())
{
  setZoomerListener(std::make_shared<app::AppZoomer>());
}

void UIView::onRender(SkCanvas* canvas)
{
  if (m_drawGrayBackground)
  {
    if (auto page = currentPage())
    {
      canvas->clear(page->backgroundColor());
    }
  }
  AppScene::onRender(canvas);
  setDirty(false);
}

bool UIView::onEvent(UEvent evt, void* userData)
{
  if (m_isZoomerEnabled && AppScene::onEvent(evt, userData))
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
  UIEvent::TargetIdType       targetId;
  UIEvent::TargetNameType     targetName;
  std::shared_ptr<LayoutNode> targetNode;
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

      m_possibleClickTargetNode = nullptr;
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
          targetId,
          targetName,
          evt.key.keysym.sym,
          evt.key.repeat,
          alt,
          ctrl,
          meta,
          shift)),
        targetNode);
    }
    break;

    case VGG_KEYUP:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
      m_eventListener(
        UIEventPtr(new KeyboardEvent(
          EUIEventType::KEYUP,
          targetId,
          targetName,
          evt.key.keysym.sym,
          evt.key.repeat,
          alt,
          ctrl,
          meta,
          shift)),
        targetNode);
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
  if (m_zoomerListener)
  {
    auto offset = m_zoomerListener->translate();
    point.x -= offset.x;
    point.y -= offset.y;

    auto scaleFactor = m_zoomerListener->scale();
    point.x /= scaleFactor;
    point.y /= scaleFactor;
  }

  return point;
}

void UIView::nextArtboard()
{
  setCurrentPage(m_page + 1);
}

void UIView::preArtboard()
{
  setCurrentPage(m_page - 1);
}

std::shared_ptr<LayoutNode> UIView::currentPage()
{
  auto document = m_document.lock();
  if (document)
  {
    return document->children()[m_page];
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

  if (handleMouseEventOnPage(page, jsButtonIndex, x, y, motionX, motionY, type))
  {
    return true;
  }

  const auto& currentPageId = page->id();
  if (m_presentingPages.contains(currentPageId))
  {
    const auto& fromPageId = m_presentingPages[currentPageId];
    if (const auto& fromPage = pageById(fromPageId))
    {
      return handleMouseEventOnPage(page, jsButtonIndex, x, y, motionX, motionY, type);
    }
  }

  return false;
}

bool UIView::handleMouseEventOnPage(
  std::shared_ptr<LayoutNode> page,
  int                         jsButtonIndex,
  int                         x,
  int                         y,
  int                         motionX,
  int                         motionY,
  EUIEventType                type)
{
  Layout::Point pointToPage =
    converPointFromWindowAndScale({ TO_VGG_LAYOUT_SCALAR(x), TO_VGG_LAYOUT_SCALAR(y) });
  Layout::Point pointToDocument{ pointToPage.x + page->frame().origin.x,
                                 pointToPage.y + page->frame().origin.y };

  const auto& target = page->hitTest(
    pointToDocument,
    [&queryHasEventListener = m_hasEventListener, type](const std::string& targetKey)
    { return queryHasEventListener(targetKey, type); });
  std::shared_ptr<VGG::LayoutNode> hitNodeInTarget;
  if (target)
  {
    hitNodeInTarget = target->hitTest(pointToDocument, nullptr);
  }

  switch (type)
  {
    case EUIEventType::MOUSEDOWN:
    {
      std::shared_ptr<VGG::LayoutNode> clickTarget;

      EUIEventType types[] = { EUIEventType::CLICK,
                               EUIEventType::AUXCLICK,
                               EUIEventType::CONTEXTMENU };
      for (auto clickType : types)
      {
        clickTarget = page->hitTest(
          pointToDocument,
          [&queryHasEventListener = m_hasEventListener, clickType](const std::string& targetKey)
          { return queryHasEventListener(targetKey, clickType); });
        if (clickTarget)
        {
          m_possibleClickTargetNode = clickTarget;
          DEBUG(
            "mousedown, m_possibleClickTargetNode: %s",
            m_possibleClickTargetNode->path().c_str());
          break;
        }
      }
    }
    break;

    case EUIEventType::MOUSEOVER:
    {
      if (target)
      {
        if (target == m_mouseOverTargetNode && m_mouseOverNode == hitNodeInTarget)
        {
          return true;
        }
        m_mouseOverTargetNode = target;
        m_mouseOverNode = hitNodeInTarget;
      }
      else
      {
        m_mouseOverTargetNode = nullptr;
        m_mouseOverNode = nullptr;
      }
    }
    break;

    case EUIEventType::MOUSEENTER:
    {
      if (target && target == m_mouseEnterTargetNode)
      {
        return true;
      }
      m_mouseEnterTargetNode = target;
    }
    break;

    case EUIEventType::MOUSEOUT:
    {
      handleMouseOut(target, hitNodeInTarget, jsButtonIndex, x, y, motionX, motionY);
      return true;
    }
    break;

    case EUIEventType::MOUSELEAVE:
    {
      handleMouseLeave(target, jsButtonIndex, x, y, motionX, motionY);
      return true;
    }
    break;

    case EUIEventType::CLICK:
    case EUIEventType::AUXCLICK:
    case EUIEventType::CONTEXTMENU:
    {
      if (target && m_possibleClickTargetNode != target)
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
  std::shared_ptr<VGG::LayoutNode> target,
  std::shared_ptr<VGG::LayoutNode> hitNodeInTarget,
  int                              jsButtonIndex,
  int                              x,
  int                              y,
  int                              motionX,
  int                              motionY)
{
  std::shared_ptr<VGG::LayoutNode> fireTarget;

  if (target)
  {
    if (m_mouseOutTargetNode)
    {
      if (target == m_mouseOutTargetNode)
      {
        if (m_mouseOutNode != hitNodeInTarget)
        {
          fireTarget = m_mouseOutTargetNode;
          m_mouseOutNode = hitNodeInTarget;
        }
      }
      else
      {
        fireTarget = m_mouseOutTargetNode;
        m_mouseOutTargetNode = target;
        m_mouseOutNode = hitNodeInTarget;
      }
    }
    else
    {
      DEBUG("m_mouseOutTargetNode: %s", fireTarget->path().c_str());
      m_mouseOutTargetNode = target;
      m_mouseOutNode = hitNodeInTarget;
      return;
    }
  }
  else
  {
    if (m_mouseOutTargetNode)
    {
      fireTarget = m_mouseOutTargetNode;
      DEBUG("mouse out of the m_mouseOutTargetNode, set it to nullptr");
      m_mouseOutTargetNode = nullptr;
      m_mouseOutNode = nullptr;
    }
  }

  if (fireTarget)
  {
    fireMouseEvent(fireTarget, EUIEventType::MOUSEOUT, jsButtonIndex, x, y, motionX, motionY);
  }
}

void UIView::handleMouseLeave(
  std::shared_ptr<VGG::LayoutNode> target,
  int                              jsButtonIndex,
  int                              x,
  int                              y,
  int                              motionX,
  int                              motionY)
{
  std::shared_ptr<VGG::LayoutNode> fireTarget;

  if (target)
  {
    if (m_mouseLeaveTargetNode)
    {
      if (target == m_mouseLeaveTargetNode)
      {
        return;
      }
      else
      {
        fireTarget = m_mouseLeaveTargetNode;
      }
    }

    m_mouseLeaveTargetNode = target;
  }
  else
  {
    if (m_mouseLeaveTargetNode)
    {
      fireTarget = m_mouseLeaveTargetNode;
      m_mouseLeaveTargetNode = nullptr;
    }
  }

  if (fireTarget)
  {
    fireMouseEvent(fireTarget, EUIEventType::MOUSELEAVE, jsButtonIndex, x, y, motionX, motionY);
  }
}

void UIView::fireMouseEvent(
  std::shared_ptr<VGG::LayoutNode> target,
  VGG::EUIEventType                type,
  int                              jsButtonIndex,
  int                              x,
  int                              y,
  int                              motionX,
  int                              motionY)
{
  auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
  m_eventListener(
    UIEventPtr(new MouseEvent(
      type,
      target ? target->id() : K_EMPTY_STRING,
      target ? target->name() : K_EMPTY_STRING,
      jsButtonIndex,
      x,
      y,
      motionX,
      motionY,
      alt,
      ctrl,
      meta,
      shift)),
    target);
}

void UIView::show(const ViewModel& viewModel, bool force)
{
  if (m_skipUntilNextLoop && !force)
  {
    return;
  }

  std::vector<layer::StructFrameObject> frames;
  for (auto& f : *viewModel.designDoc())
  {
    if (f->type() == VGG::Domain::Element::EType::FRAME)
    {
      frames.emplace_back(layer::StructFrameObject(f.get()));
    }
  }
  auto result =
    layer::SceneBuilder::builder().setResetOriginEnable(true).build<layer::StructModelFrame>(
      std::move(frames));
  if (result.root)
  {
    show(viewModel, std::move(*result.root), force);
  }
  else
  {
    WARN("#UIView::show, built scene is empty");
  }
}

void UIView::show(const ViewModel& viewModel, std::vector<layer::FramePtr> frames, bool force)
{
  if (m_skipUntilNextLoop && !force)
  {
    return;
  }
  m_skipUntilNextLoop = true;

  setSceneRoots(frames);
  Scene::setPage(m_page);

  m_document = viewModel.layoutTree();
  // todo, merge edited doc resouces ?
  Scene::setResRepo(viewModel.resources());

  setDirty(true);
}

void UIView::fitContent(float xOffset, float yOffset, float scale)
{
  ASSERT(m_zoomerListener);

  m_zoomerListener->setScale(scale);
  m_zoomerListener->setOffset({ xOffset, yOffset });
}

void UIView::fitCurrentPage()
{
  ASSERT(m_zoomerListener);

  m_zoomerListener->setScale(Zoomer::SL_1_1);
  m_zoomerListener->setOffset({ 0, 0 });
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

  auto target = page->hitTest(
    pointToDocument,
    [&queryHasEventListener = m_hasEventListener, type](const std::string& targetKey)
    { return queryHasEventListener(targetKey, type); });

  m_eventListener(
    UIEventPtr(new TouchEvent(
      type,
      target ? target->id() : K_EMPTY_STRING,
      target ? target->name() : K_EMPTY_STRING)),
    target);

  return true;
}

UIView::Offset UIView::getOffset()
{
  auto offset = m_zoomerListener->translate();
  return { offset.x, offset.y };
}

void UIView::setOffset(Offset offset)
{
  m_zoomerListener->setOffset(glm::vec2{ offset.x, offset.y });
  setDirty(true);
}

bool UIView::setCurrentPage(int index)
{
  auto oldPage = m_page;
  bool success = setCurrentPageIndex(index);

  if (m_page != oldPage)
  {
    m_history.push(currentPage()->id());
  }

  return success;
}

bool UIView::back()
{
  if (m_history.size() <= 1)
  {
    return false;
  }

  auto displayedPage = currentPage();

  m_history.pop();
  auto newPageId = m_history.top();

  auto document = m_document.lock();
  if (document)
  {
    for (std::size_t index = 0; index < document->children().size(); ++index)
    {
      if (document->children()[index]->id() == newPageId)
      {
        // clear presented page relations
        auto to = displayedPage->id();
        while (!to.empty())
        {
          if (!m_presentingPages.contains(to))
          {
            break;
          }

          auto from = m_presentingPages[to];
          m_presentingPages.erase(to);
          m_presentedPages.erase(from);

          to = from;
        }

        setCurrentPageIndex(index);
        return true;
      }
    }
  }

  return false;
}

bool UIView::setCurrentPageIndex(int index)
{
  if (m_page == index)
  {
    return true;
  }

  if (auto document = m_document.lock())
  {
    if (index >= 0 && static_cast<std::size_t>(index) < document->children().size())
    {
      INFO("show page: %d, %s", index, document->children()[index]->name().c_str());
      m_page = index;
      if (index > Scene::currentPage())
      {
        auto offset = index - Scene::currentPage();
        while (offset-- > 0)
        {
          Scene::nextArtboard();
        }
      }
      else
      {
        auto offset = Scene::currentPage() - index;
        while (offset-- > 0)
        {
          Scene::preArtboard();
        }
      }
      setDirty(true);
      return true;
    }
  }
  return false;
}

bool UIView::presentPage(int index)
{
  auto from = currentPage()->id();
  auto oldPageIndex = m_page;

  setCurrentPageIndex(index);
  if (m_page != oldPageIndex)
  {
    auto to = currentPage()->id();
    m_presentedPages[from] = to;
    m_presentingPages[to] = from;
    return true;
  }

  return false;
}

bool UIView::dismissPage()
{
  auto to = currentPage()->id();

  if (!m_presentingPages.contains(to))
  {
    return false;
  }

  auto from = m_presentingPages[to];
  m_presentingPages.erase(to);
  m_presentedPages.erase(from);

  auto document = m_document.lock();
  if (document)
  {
    for (std::size_t index = 0; index < document->children().size(); ++index)
    {
      if (document->children()[index]->id() == from)
      {
        return setCurrentPageIndex(index);
      }
    }
  }

  return false;
}
