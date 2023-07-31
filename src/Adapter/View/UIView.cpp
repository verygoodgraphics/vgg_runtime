#include "Application/UIView.hpp"

#include "Scene/Zoomer.h"

#include <SDL2/SDL.h>

using namespace VGG;

void UIView::onEvent(const SDL_Event& evt)
{
  if (!m_event_listener)
  {
    return;
  }

  // todo, handle zoom

  // todo, hittest
  for (auto& subview : m_subviews)
  {
    subview->onEvent(evt);
  }

  // todo, capturing
  // todo, bubbling
  UIEvent::PathType target_path{ "/fake/update_background_color" };

  switch (evt.type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      auto js_button_index{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
      m_event_listener(UIEventPtr(new MouseEvent(target_path,
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
    break;

    case SDL_MOUSEMOTION:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
      m_event_listener(UIEventPtr(new MouseEvent(target_path,
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
    break;

    case SDL_MOUSEBUTTONUP:
    {
      auto js_button_index{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
      m_event_listener(UIEventPtr(new MouseEvent(target_path,
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

      if (js_button_index == 0)
      {
        m_event_listener(UIEventPtr(new MouseEvent(target_path,
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
      else
      {
        m_event_listener(UIEventPtr(new MouseEvent(target_path,
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

        if (js_button_index == 2)
        {
          m_event_listener(UIEventPtr(new MouseEvent(target_path,
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
  auto is_editor = !m_self_zoom_enabled;

  if (!is_editor) // zoom self & subviews
  {
    zoomer->apply(canvas);
    m_scene->render(canvas);
  }
  else // zoom only subviews
  {
    m_scene->render(canvas); // render self(editor) without zoom

    // setup clip & offset for edit view
    SkRect edit_rect{ SkIntToScalar(m_left),
                      SkIntToScalar(m_top),
                      SkIntToScalar(m_width - m_right),
                      SkIntToScalar(m_height - m_bottom) };
    canvas->clipRect(edit_rect);
    canvas->translate(SkIntToScalar(m_left), SkIntToScalar(m_top));

    zoomer->apply(canvas);
  }

  for (auto& subview : m_subviews)
  {
    subview->draw(canvas, zoomer);
  }

  // restore zoom
  zoomer->restore(canvas);
  // restore offset for edit view
  if (is_editor)
  {
    canvas->translate(-SkIntToScalar(m_left), -SkIntToScalar(m_top));
  }
}

void UIView::becomeEditorWithSidebar(scalar_type top,
                                     scalar_type right,
                                     scalar_type bottom,
                                     scalar_type left)
{
  m_self_zoom_enabled = false;

  m_top = top;
  m_right = right;
  m_bottom = bottom;
  m_left = left;

  // todo, editor layout
}