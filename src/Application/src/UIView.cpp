#include "UIView.hpp"

#include "Event/Event.h"
#include "Event/EventAPI.h"
#include "Event/Keycode.h"

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

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
  switch (evt.type)
  {
    case VGG_MOUSEBUTTONDOWN:
    {
      if (!m_root)
      {
        return false;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.button.windowX),
                           TO_VGG_LAYOUT_SCALAR(evt.button.windowY) };
      point = converPointFromWindowAndScale(point);
      auto targetNode = m_root->hitTest(point,
                                        [&hasEventListener](const std::string& path)
                                        { return hasEventListener(path, UIEventType::mousedown); });
      if (targetNode)
      {
        auto jsButtonIndex{ evt.button.button - 1 };
        auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
        m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                  UIEventType::mousedown,
                                                  jsButtonIndex,
                                                  evt.button.windowX,
                                                  evt.button.windowY,
                                                  0,
                                                  0,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)));
      }
    }
    break;

    case VGG_MOUSEMOTION:
    {
      if (!m_root)
      {
        return false;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.motion.windowX),
                           TO_VGG_LAYOUT_SCALAR(evt.motion.windowY) };
      point = converPointFromWindowAndScale(point);
      auto targetNode = m_root->hitTest(point,
                                        [&hasEventListener](const std::string& path)
                                        { return hasEventListener(path, UIEventType::mousemove); });
      if (targetNode)
      {
        auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());
        m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                  UIEventType::mousemove,
                                                  0,
                                                  evt.motion.windowX,
                                                  evt.motion.windowY,
                                                  evt.motion.xrel,
                                                  evt.motion.yrel,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)));
      }
    }
    break;

    case VGG_MOUSEBUTTONUP:
    {
      if (!m_root)
      {
        return false;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.button.windowX),
                           TO_VGG_LAYOUT_SCALAR(evt.button.windowY) };
      point = converPointFromWindowAndScale(point);
      auto jsButtonIndex{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(EventManager::getModState());

      auto targetNode = m_root->hitTest(point,
                                        [&hasEventListener](const std::string& path)
                                        { return hasEventListener(path, UIEventType::mouseup); });
      if (targetNode)
      {
        m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                  UIEventType::mouseup,
                                                  jsButtonIndex,
                                                  evt.button.windowX,
                                                  evt.button.windowY,
                                                  0,
                                                  0,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)));
      }

      if (jsButtonIndex == 0)
      {
        auto targetNode = m_root->hitTest(point,
                                          [&hasEventListener](const std::string& path)
                                          { return hasEventListener(path, UIEventType::click); });
        if (targetNode)
        {
          m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                    UIEventType::click,
                                                    jsButtonIndex,
                                                    evt.button.windowX,
                                                    evt.button.windowY,
                                                    0,
                                                    0,
                                                    alt,
                                                    ctrl,
                                                    meta,
                                                    shift)));
        }
      }
      else
      {
        auto targetNode = m_root->hitTest(point,
                                          [&hasEventListener](const std::string& path) {
                                            return hasEventListener(path, UIEventType::auxclick);
                                          });
        if (targetNode)
        {
          m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                    UIEventType::auxclick,
                                                    jsButtonIndex,
                                                    evt.button.windowX,
                                                    evt.button.windowY,
                                                    0,
                                                    0,
                                                    alt,
                                                    ctrl,
                                                    meta,
                                                    shift)));
        }

        if (jsButtonIndex == 2)
        {
          auto targetNode =
            m_root->hitTest(point,
                            [&hasEventListener](const std::string& path)
                            { return hasEventListener(path, UIEventType::contextmenu); });
          if (targetNode)
          {
            m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                      UIEventType::contextmenu,
                                                      jsButtonIndex,
                                                      evt.button.windowX,
                                                      evt.button.windowY,
                                                      0,
                                                      0,
                                                      alt,
                                                      ctrl,
                                                      meta,
                                                      shift)));
          }
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
                                                   shift)));
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
                                                   shift)));
    }
    break;

    case VGG_FINGERDOWN:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchstart)));
    }
    break;

    case VGG_FINGERMOTION:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchmove)));
    }
    break;

    case VGG_FINGERUP:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchend)));
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
