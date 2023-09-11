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

  // todo, hittest
  // todo, select page
  for (auto& subview : m_subviews)
  {
    subview->onEvent(evt, userData);
  }

  // todo, capturing
  // todo, bubbling
  UIEvent::PathType targetPath{ "/fake/update_background_color" };

  decltype(m_hasEventListener) hasEventListener = [this](const std::string& path, UIEventType type)
  {
    if (this->m_superview && this->m_superview->m_isEditor)
    {
      return true;
    }
    else
    {
      return m_hasEventListener(path, type);
    }
  };

  std::shared_ptr<LayoutNode> targetNode;
  switch (evt.type)
  {
    case VGG_MOUSEBUTTONDOWN:
    {
      auto page = currentPage();
      if (!page)
      {
        return false;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.button.windowX),
                           TO_VGG_LAYOUT_SCALAR(evt.button.windowY) };
      point = converPointFromWindowAndScale(point);
      targetNode = page->hitTest(point,
                                 [&hasEventListener](const std::string& path)
                                 { return hasEventListener(path, UIEventType::mousedown); });
      auto jsButtonIndex{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
      m_eventListener(UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                                UIEventType::mousedown,
                                                jsButtonIndex,
                                                evt.button.windowX,
                                                evt.button.windowY,
                                                0,
                                                0,
                                                alt,
                                                ctrl,
                                                meta,
                                                shift)),
                      targetNode);
    }
    break;

    case VGG_MOUSEMOTION:
    {
      auto page = currentPage();
      if (!page)
      {
        return false;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.motion.windowX),
                           TO_VGG_LAYOUT_SCALAR(evt.motion.windowY) };
      point = converPointFromWindowAndScale(point);
      targetNode = page->hitTest(point,
                                 [&hasEventListener](const std::string& path)
                                 { return hasEventListener(path, UIEventType::mousemove); });
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
      m_eventListener(UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                                UIEventType::mousemove,
                                                0,
                                                evt.motion.windowX,
                                                evt.motion.windowY,
                                                evt.motion.xrel,
                                                evt.motion.yrel,
                                                alt,
                                                ctrl,
                                                meta,
                                                shift)),
                      targetNode);
    }
    break;

    case VGG_MOUSEBUTTONUP:
    {
      auto page = currentPage();
      if (!page)
      {
        return false;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.button.windowX),
                           TO_VGG_LAYOUT_SCALAR(evt.button.windowY) };
      point = converPointFromWindowAndScale(point);
      auto jsButtonIndex{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());

      targetNode = page->hitTest(point,
                                 [&hasEventListener](const std::string& path)
                                 { return hasEventListener(path, UIEventType::mouseup); });
      m_eventListener(UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                                UIEventType::mouseup,
                                                jsButtonIndex,
                                                evt.button.windowX,
                                                evt.button.windowY,
                                                0,
                                                0,
                                                alt,
                                                ctrl,
                                                meta,
                                                shift)),
                      targetNode);

      if (jsButtonIndex == 0)
      {
        targetNode = page->hitTest(point,
                                   [&hasEventListener](const std::string& path)
                                   { return hasEventListener(path, UIEventType::click); });
        m_eventListener(UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                                  UIEventType::click,
                                                  jsButtonIndex,
                                                  evt.button.windowX,
                                                  evt.button.windowY,
                                                  0,
                                                  0,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)),
                        targetNode);
      }
      else
      {
        targetNode = page->hitTest(point,
                                   [&hasEventListener](const std::string& path)
                                   { return hasEventListener(path, UIEventType::auxclick); });
        m_eventListener(UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                                  UIEventType::auxclick,
                                                  jsButtonIndex,
                                                  evt.button.windowX,
                                                  evt.button.windowY,
                                                  0,
                                                  0,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)),
                        targetNode);

        if (jsButtonIndex == 2)
        {
          targetNode = page->hitTest(point,
                                     [&hasEventListener](const std::string& path)
                                     { return hasEventListener(path, UIEventType::contextmenu); });
          m_eventListener(
            UIEventPtr(new MouseEvent(targetNode ? targetNode->path() : K_EMPTY_STRING,
                                      UIEventType::contextmenu,
                                      jsButtonIndex,
                                      evt.button.windowX,
                                      evt.button.windowY,
                                      0,
                                      0,
                                      alt,
                                      ctrl,
                                      meta,
                                      shift)),
            targetNode);
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

Layout::Point UIView::converPointFromWindow(Layout::Point point)
{
  auto x = point.x;
  auto y = point.y;

  auto parent = this;
  while (parent)
  {
    x -= parent->m_frame.origin.x;
    y -= parent->m_frame.origin.y;

    x -= parent->m_bounds.origin.x;
    y -= parent->m_bounds.origin.y;

    parent = parent->m_superview;
  }

  if (m_zoomerListener)
  {
    auto offset = m_zoomerListener->translate();
    x -= offset.x;
    y -= offset.y;
  }

  auto page = currentPage();
  if (page)
  {
    x += page->frame().origin.x;
    y += page->frame().origin.y;
  }

  return { x, y };
}

Layout::Point UIView::converPointFromWindowAndScale(Layout::Point point)
{
  point = converPointFromWindow(point);
  if (m_zoomerListener)
  {
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
