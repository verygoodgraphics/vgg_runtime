#include "Application/UIView.hpp"
#include "Application/interface/Event/Event.h"

#include "Scene/Zoomer.h"

#include <SDL2/SDL.h>

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

void UIView::onEvent(const SDL_Event& evt, Zoomer* zoomer)
{
  if (!m_event_listener)
  {
    return;
  }

  if (!m_is_editor)
  {
    m_bounds.origin.x = zoomer->offset.x;
    m_bounds.origin.y = zoomer->offset.y;

    m_contentScaleFactor = zoomer->zoom;
  }

  // todo, hittest
  for (auto& subview : m_subviews)
  {
    subview->onEvent(evt, zoomer);
  }

  // todo, capturing
  // todo, bubbling
  UIEvent::PathType target_path{ "/fake/update_background_color" };

  decltype(m_has_event_listener) has_event_listener =
    [this](const std::string& path, UIEventType type)
  {
    if (this->m_superview && this->m_superview->m_is_editor)
    {
      return true;
    }
    else
    {
      return m_has_event_listener(path, type);
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
      auto target_view = m_root->hitTest(point,
                                         [&has_event_listener](const std::string& path) {
                                           return has_event_listener(path, UIEventType::mousedown);
                                         });
      if (target_view)
      {
        auto js_button_index{ evt.button.button - 1 };
        auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
        m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                   UIEventType::mousedown,
                                                   js_button_index,
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
      auto target_view = m_root->hitTest(point,
                                         [&has_event_listener](const std::string& path) {
                                           return has_event_listener(path, UIEventType::mousemove);
                                         });
      if (target_view)
      {
        auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
        m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
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
      auto js_button_index{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());

      auto target_view = m_root->hitTest(point,
                                         [&has_event_listener](const std::string& path) {
                                           return has_event_listener(path, UIEventType::mouseup);
                                         });
      if (target_view)
      {
        m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                   UIEventType::mouseup,
                                                   js_button_index,
                                                   evt.button.x,
                                                   evt.button.y,
                                                   0,
                                                   0,
                                                   alt,
                                                   ctrl,
                                                   meta,
                                                   shift)));
      }

      if (js_button_index == 0)
      {
        auto target_view = m_root->hitTest(point,
                                           [&has_event_listener](const std::string& path) {
                                             return has_event_listener(path, UIEventType::click);
                                           });
        if (target_view)
        {
          m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                     UIEventType::click,
                                                     js_button_index,
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
        auto target_view = m_root->hitTest(point,
                                           [&has_event_listener](const std::string& path) {
                                             return has_event_listener(path, UIEventType::auxclick);
                                           });
        if (target_view)
        {
          m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                     UIEventType::auxclick,
                                                     js_button_index,
                                                     evt.button.x,
                                                     evt.button.y,
                                                     0,
                                                     0,
                                                     alt,
                                                     ctrl,
                                                     meta,
                                                     shift)));
        }

        if (js_button_index == 2)
        {
          auto target_view =
            m_root->hitTest(point,
                            [&has_event_listener](const std::string& path)
                            { return has_event_listener(path, UIEventType::contextmenu); });
          if (target_view)
          {
            m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                       UIEventType::contextmenu,
                                                       js_button_index,
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
      m_event_listener(UIEventPtr(new KeyboardEvent(target_path,
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
      m_event_listener(UIEventPtr(new KeyboardEvent(target_path,
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
      m_event_listener(UIEventPtr(new TouchEvent(target_path, UIEventType::touchstart)));
    }
    break;

    case SDL_FINGERMOTION:
    {
      m_event_listener(UIEventPtr(new TouchEvent(target_path, UIEventType::touchmove)));
    }
    break;

    case SDL_FINGERUP:
    {
      m_event_listener(UIEventPtr(new TouchEvent(target_path, UIEventType::touchend)));
    }
    break;

      // todo, more event

    default:
      break;
  }
}

std::tuple<bool, bool, bool, bool> UIView::getKeyModifier(int keyMod)
{
  bool l_alt = keyMod & KMOD_LALT;
  bool l_ctrl = keyMod & KMOD_LCTRL;
  bool l_meta = keyMod & KMOD_LGUI;
  bool l_shift = keyMod & KMOD_LSHIFT;

  bool r_alt = keyMod & KMOD_RALT;
  bool r_ctrl = keyMod & KMOD_RCTRL;
  bool r_meta = keyMod & KMOD_RGUI;
  bool r_shift = keyMod & KMOD_RSHIFT;

  return { l_alt || r_alt, l_ctrl || r_ctrl, l_meta || r_meta, l_shift || r_shift };
}

void UIView::draw(SkCanvas* canvas, Zoomer* zoomer)
{

  UEvent e;
  e.type = VGG_PAINT;
  e.paint.data = canvas;
  // DEPRECATED: VPaintEvent will be removed from UEvent,
  // the only way to paint a scene is added in
  // a layer
  if (m_is_editor) // editor; zoom only subviews
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
    SkRect edit_rect{ 0, 0, m_frame.size.width, m_frame.size.height };
    canvas->clipRect(edit_rect);

    zoomer->apply(canvas);

    m_scene->render(canvas);

    canvas->restore();
  }
}

void UIView::becomeEditorWithSidebar(scalar_type top,
                                     scalar_type right,
                                     scalar_type bottom,
                                     scalar_type left)
{
  m_is_editor = true;

  m_top = top;
  m_right = right;
  m_bottom = bottom;
  m_left = left;

  // todo, editor layout
}

void UIView::layoutSubviews()
{
  if (m_is_editor)
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
  if (!m_is_editor && SDL_GetModState() & KMOD_CTRL)
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

    m_is_dirty = true;
  }
}
