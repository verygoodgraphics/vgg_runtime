/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Layer/Core/RasterCacheTile.hpp"
#include "Layer/Zoomer.hpp"
#include "ViewModel.hpp"

#include "Layer/SceneBuilder.hpp"
#include "Utility/Log.hpp"

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

constexpr auto K_EMPTY_STRING = "";

UIView::UIView()
  : AppScene(std::make_unique<VGG::layer::RasterCacheTile>(1024, 1024))
{
  setZoomerListener(std::make_shared<app::AppZoomer>());
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
  UIEvent::TargetPathType     targetPath;
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
      return handleMouseEvent(
        0,
        evt.motion.windowX,
        evt.motion.windowY,
        evt.motion.xrel,
        evt.motion.yrel,
        EUIEventType::MOUSEMOVE);
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
          targetPath,
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
          targetPath,
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
  auto document = m_document.lock();
  if (document)
  {
    if (m_page >= (int)document->children().size() - 1)
    {
      return;
    }

    ++m_page;
    Scene::nextArtboard();
    setDirty(true);
  }
}

void UIView::preArtboard()
{
  auto document = m_document.lock();
  if (document)
  {
    if (m_page <= 0)
    {
      return;
    }

    --m_page;
    Scene::preArtboard();
    setDirty(true);
  }
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

  Layout::Point pointToPage =
    converPointFromWindowAndScale({ TO_VGG_LAYOUT_SCALAR(x), TO_VGG_LAYOUT_SCALAR(y) });
  Layout::Point pointToDocument{ pointToPage.x + page->frame().origin.x,
                                 pointToPage.y + page->frame().origin.y };

  auto target = page->hitTest(
    pointToDocument,
    [&queryHasEventListener = m_hasEventListener, type](const std::string& targetKey)
    { return queryHasEventListener(targetKey, type); });
  auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());

  m_eventListener(
    UIEventPtr(new MouseEvent(
      type,
      target ? target->vggId() : K_EMPTY_STRING,
      target ? target->path() : K_EMPTY_STRING,
      jsButtonIndex,
      pointToPage.x,
      pointToPage.y,
      motionX,
      motionY,
      alt,
      ctrl,
      meta,
      shift)),
    target);

  return true;
}

void UIView::show(const ViewModel& viewModel)
{
  if (m_skipUntilNextLoop)
  {
    return;
  }

  auto result = layer::SceneBuilder::builder()
                  .setResetOriginEnable(true)
                  .setDoc(viewModel.designDoc()->content())
                  .build();
  if (result.root)
  {
    show(viewModel, std::move(*result.root));
  }
  else
  {
    WARN("#UIView::show, built scene is empty");
  }
}

void UIView::show(const ViewModel& viewModel, std::vector<FramePtr> frames)
{
  if (m_skipUntilNextLoop)
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

  m_zoomerListener->setOffset({ xOffset, yOffset });
  m_zoomerListener->setScale(scale);
}

void UIView::fitCurrentPage()
{
  ASSERT(m_zoomerListener);

  m_zoomerListener->setOffset({ 0, 0 });
  m_zoomerListener->setScale(Zoomer::SL_1_1);
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
      target ? target->vggId() : K_EMPTY_STRING,
      target ? target->path() : K_EMPTY_STRING)),
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
  if (auto document = m_document.lock())
  {
    if (index >= 0 && static_cast<std::size_t>(index) < document->children().size())
    {
      m_page = index;
      Scene::setPage(index);
      setDirty(true);
      return true;
    }
  }
  return false;
}
