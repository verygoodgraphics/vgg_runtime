#include "UIView.hpp"

#include "Event/Event.h"
#include "Event/EventAPI.h"
#include "Event/Keycode.h"

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

constexpr auto K_EMPTY_STRING = "";

bool UIView::onEvent(UEvent evt, void* userData)
{
  if (AppScene::onEvent(evt, userData))
  {
    return true;
  }

  if (!m_eventListener)
  {
    return false;
  }

  // todo, capturing
  // todo, bubbling
  UIEvent::PathType targetPath;
  auto& hasEventListener = m_hasEventListener;
  std::shared_ptr<LayoutNode> targetNode;
  switch (evt.type)
  {
    case VGG_MOUSEBUTTONDOWN:
    {
      auto jsButtonIndex{ evt.button.button - 1 };
      return handleMouseEvent(jsButtonIndex,
                              evt.button.windowX,
                              evt.button.windowY,
                              0,
                              0,
                              UIEventType::mousedown);
    }
    break;

    case VGG_MOUSEMOTION:
    {
      return handleMouseEvent(0,
                              evt.motion.windowX,
                              evt.motion.windowY,
                              evt.motion.xrel,
                              evt.motion.yrel,
                              UIEventType::mousemove);
    }
    break;

    case VGG_MOUSEBUTTONUP:
    {
      auto jsButtonIndex{ evt.button.button - 1 };

      handleMouseEvent(jsButtonIndex,
                       evt.button.windowX,
                       evt.button.windowY,
                       0,
                       0,
                       UIEventType::mouseup);

      if (jsButtonIndex == 0)
      {
        handleMouseEvent(jsButtonIndex,
                         evt.button.windowX,
                         evt.button.windowY,
                         0,
                         0,
                         UIEventType::click);
      }
      else
      {
        handleMouseEvent(jsButtonIndex,
                         evt.button.windowX,
                         evt.button.windowY,
                         0,
                         0,
                         UIEventType::auxclick);

        if (jsButtonIndex == 2)
        {
          handleMouseEvent(jsButtonIndex,
                           evt.button.windowX,
                           evt.button.windowY,
                           0,
                           0,
                           UIEventType::contextmenu);
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
      m_eventListener(UIEventPtr(new KeyboardEvent(targetPath,
                                                   UIEventType::keydown,
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
      m_eventListener(UIEventPtr(new KeyboardEvent(targetPath,
                                                   UIEventType::keyup,
                                                   evt.key.keysym.sym,
                                                   evt.key.repeat,
                                                   alt,
                                                   ctrl,
                                                   meta,
                                                   shift)),
                      targetNode);
    }
    break;

    case VGG_FINGERDOWN:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchstart)), targetNode);
    }
    break;

    case VGG_FINGERMOTION:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchmove)), targetNode);
    }
    break;

    case VGG_FINGERUP:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchend)), targetNode);
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

void UIView::becomeEditorWithSidebar(ScalarType top,
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

Layout::Point UIView::converPointFromWindowAndScale(Layout::Point point)
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
    if (m_page >= document->children().size() - 1)
    {
      return;
    }

    ++m_page;
    Scene::nextArtboard();
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

bool UIView::handleMouseEvent(int jsButtonIndex,
                              int x,
                              int y,
                              int motionX,
                              int motionY,
                              UIEventType type)
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

  auto targetNode =
    page->hitTest(pointToDocument,
                  [&queryHasEventListener = m_hasEventListener, type](const std::string& path)
                  { return queryHasEventListener(path, type); });
  auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());

  m_eventListener(UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                            type,
                                            jsButtonIndex,
                                            pointToPage.x,
                                            pointToPage.y,
                                            motionX,
                                            motionY,
                                            alt,
                                            ctrl,
                                            meta,
                                            shift)),
                  targetNode);

  return true;
}