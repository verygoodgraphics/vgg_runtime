#include "Application/UIView.hpp"
#include "Application/interface/Event/Event.h"

#include "Scene/Zoomer.h"

#include "core/SkCanvas.h"

#include <SDL2/SDL.h>

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

void UIView::onEvent(const SDL_Event& evt, Zoomer* zoomer)
{
  if (!m_eventListener)
  {
    return;
  }

  if (!m_isEditor)
  {
    m_bounds.origin.x = zoomer->offset.x;
    m_bounds.origin.y = zoomer->offset.y;

    m_contentScaleFactor = zoomer->zoom;
  }

  // todo, hittest
  // todo, select page
  for (auto& subview : m_subviews)
  {
    subview->onEvent(evt, zoomer);
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
    case SDL_MOUSEBUTTONDOWN:
    {
      if (!m_root)
      {
        return;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.button.x), TO_VGG_LAYOUT_SCALAR(evt.button.y) };
      point = converPointFromWindowAndScale(point);
      auto targetNode = m_root->hitTest(point,
                                        [&hasEventListener](const std::string& path)
                                        { return hasEventListener(path, UIEventType::mousedown); });
      if (targetNode)
      {
        auto jsButtonIndex{ evt.button.button - 1 };
        auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
        m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                  UIEventType::mousedown,
                                                  jsButtonIndex,
                                                  evt.button.x,
                                                  evt.button.y,
                                                  0,
                                                  0,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)));
      }
    }
    break;

    case SDL_MOUSEMOTION:
    {
      if (!m_root)
      {
        return;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.motion.x), TO_VGG_LAYOUT_SCALAR(evt.motion.y) };
      point = converPointFromWindowAndScale(point);
      auto targetNode = m_root->hitTest(point,
                                        [&hasEventListener](const std::string& path)
                                        { return hasEventListener(path, UIEventType::mousemove); });
      if (targetNode)
      {
        auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
        m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                  UIEventType::mousemove,
                                                  0,
                                                  evt.motion.x,
                                                  evt.motion.y,
                                                  evt.motion.xrel,
                                                  evt.motion.yrel,
                                                  alt,
                                                  ctrl,
                                                  meta,
                                                  shift)));
      }
    }
    break;

    case SDL_MOUSEBUTTONUP:
    {
      if (!m_root)
      {
        return;
      }

      Layout::Point point{ TO_VGG_LAYOUT_SCALAR(evt.button.x), TO_VGG_LAYOUT_SCALAR(evt.button.y) };
      point = converPointFromWindowAndScale(point);
      auto jsButtonIndex{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());

      auto targetNode = m_root->hitTest(point,
                                        [&hasEventListener](const std::string& path)
                                        { return hasEventListener(path, UIEventType::mouseup); });
      if (targetNode)
      {
        m_eventListener(UIEventPtr(new MouseEvent(targetNode->path(),
                                                  UIEventType::mouseup,
                                                  jsButtonIndex,
                                                  evt.button.x,
                                                  evt.button.y,
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
                                                    evt.button.x,
                                                    evt.button.y,
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
                                                    evt.button.x,
                                                    evt.button.y,
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
                                                      evt.button.x,
                                                      evt.button.y,
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

    case SDL_MOUSEWHEEL:
    {
      handleMouseWheel(evt, zoomer);
    }
    break;

    case SDL_KEYDOWN:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
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

    case SDL_KEYUP:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
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

    case SDL_FINGERDOWN:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchstart)));
    }
    break;

    case SDL_FINGERMOTION:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchmove)));
    }
    break;

    case SDL_FINGERUP:
    {
      m_eventListener(UIEventPtr(new TouchEvent(targetPath, UIEventType::touchend)));
    }
    break;

      // todo, more event

    default:
      break;
  }
}

std::tuple<bool, bool, bool, bool> UIView::getKeyModifier(int keyMod)
{
  bool lAlt = keyMod & KMOD_LALT;
  bool lCtrl = keyMod & KMOD_LCTRL;
  bool lMeta = keyMod & KMOD_LGUI;
  bool lShift = keyMod & KMOD_LSHIFT;

  bool rAlt = keyMod & KMOD_RALT;
  bool rCtrl = keyMod & KMOD_RCTRL;
  bool rMeta = keyMod & KMOD_RGUI;
  bool rShift = keyMod & KMOD_RSHIFT;

  return { lAlt || rAlt, lCtrl || rCtrl, lMeta || rMeta, lShift || rShift };
}

void UIView::draw(SkCanvas* canvas, Zoomer* zoomer)
{

  // DEPRECATED:
  // the only way to paint a scene is added in
  // a layer
  if (m_isEditor) // editor; zoom only subviews
  {
    m_scene->render(canvas);

    // draw inner edit view
    for (auto& subview : m_subviews)
    {
      subview->draw(canvas, zoomer);
    }
  }
  else // edit view; edited document; zoom self
  {
    canvas->save();

    // setup clip & offset for edit view
    canvas->translate(m_frame.origin.x, m_frame.origin.y);
    SkRect editRect{ 0, 0, m_frame.size.width, m_frame.size.height };
    canvas->clipRect(editRect);

    zoomer->apply(canvas);

    m_scene->render(canvas);

    canvas->restore();
  }
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

  return { x, y };
}

Layout::Point UIView::converPointFromWindowAndScale(Layout::Point point)
{
  point = converPointFromWindow(point);

  auto x = point.x / m_contentScaleFactor;
  auto y = point.y / m_contentScaleFactor;

  return { x, y };
}

void UIView::handleMouseWheel(const SDL_Event& evt, Zoomer* zoomer)
{
  if (!m_isEditor && SDL_GetModState() & KMOD_CTRL)
  {
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    Layout::Point point{ TO_VGG_LAYOUT_SCALAR(mx), TO_VGG_LAYOUT_SCALAR(my) };
    point = converPointFromWindow(point);
    mx = point.x;
    my = point.y;

    double dz = (evt.wheel.y > 0 ? 1.0 : -1.0) * 0.03;
    double z2 = zoomer->zoom * (1 + dz);
    if (z2 > 0.01 && z2 < 100)
    {
      zoomer->offset.x -= (mx / zoomer->dpiRatio) * dz;
      zoomer->offset.y -= (my / zoomer->dpiRatio) * dz;
      zoomer->zoom += zoomer->zoom * dz;
    }

    m_isDirty = true;
  }
}
