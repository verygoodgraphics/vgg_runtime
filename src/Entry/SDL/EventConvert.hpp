#pragma once

#include "Application/Event/Event.hpp"
#include "Application/Event/Keycode.hpp"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <cstring>

#define SWITCH_MAP_ITEM_BEGIN(var)                                                                 \
  switch (var)                                                                                     \
  {

#define SWITCH_MAP_ITEM_DEF(from, to)                                                              \
  case from:                                                                                       \
    return to;

#define SWITCH_MAP_ITEM_DEF_NULL(from) case from:;

#define SWITCH_MAP_ITEM_END(fallback)                                                              \
  default:                                                                                         \
    return fallback;                                                                               \
    }

inline EEventType toEEventType(SDL_EventType t)
{
  SWITCH_MAP_ITEM_BEGIN(t)
  SWITCH_MAP_ITEM_DEF(SDL_TEXTEDITING, VGG_TEXTEDITING)
  SWITCH_MAP_ITEM_DEF(SDL_TEXTEDITING_EXT, VGG_TEXTEDITING_EXT)
  SWITCH_MAP_ITEM_DEF(SDL_KEYDOWN, VGG_KEYDOWN)
  SWITCH_MAP_ITEM_DEF(SDL_KEYUP, VGG_KEYUP)
  SWITCH_MAP_ITEM_DEF(SDL_TEXTINPUT, VGG_TEXTINPUT)
  SWITCH_MAP_ITEM_DEF(SDL_MOUSEWHEEL, VGG_MOUSEWHEEL)
  SWITCH_MAP_ITEM_DEF(SDL_MOUSEBUTTONDOWN, VGG_MOUSEBUTTONDOWN)
  SWITCH_MAP_ITEM_DEF(SDL_MOUSEBUTTONUP, VGG_MOUSEBUTTONUP)
  SWITCH_MAP_ITEM_DEF(SDL_MOUSEMOTION, VGG_MOUSEMOTION)
  SWITCH_MAP_ITEM_DEF(SDL_USEREVENT, VGG_USEREVENT)
  SWITCH_MAP_ITEM_DEF(SDL_DROPFILE, VGG_DROPFILE)
  SWITCH_MAP_ITEM_DEF(SDL_DROPTEXT, VGG_DROPTEXT)
  SWITCH_MAP_ITEM_DEF(SDL_DROPBEGIN, VGG_DROPBEGIN)
  SWITCH_MAP_ITEM_DEF(SDL_DROPCOMPLETE, VGG_DROPCOMPLETE)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT, VGG_WINDOWEVENT)
  SWITCH_MAP_ITEM_DEF(SDL_QUIT, VGG_QUIT)
  SWITCH_MAP_ITEM_END(EEventType::VGG_USEREVENT)
}

inline EWindowEventID toEWindowEventID(SDL_WindowEventID id)
{
  SWITCH_MAP_ITEM_BEGIN(id)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_NONE, VGG_WINDOWEVENT_NONE)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_SHOWN, VGG_WINDOWEVENT_SHOWN)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_HIDDEN, VGG_WINDOWEVENT_HIDDEN)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_EXPOSED, VGG_WINDOWEVENT_EXPOSED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_RESIZED, VGG_WINDOWEVENT_RESIZED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_SIZE_CHANGED, VGG_WINDOWEVENT_SIZE_CHANGED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_MINIMIZED, VGG_WINDOWEVENT_MINIMIZED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_MAXIMIZED, VGG_WINDOWEVENT_MAXIMIZED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_RESTORED, VGG_WINDOWEVENT_RESTORED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_ENTER, VGG_WINDOWEVENT_ENTER)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_LEAVE, VGG_WINDOWEVENT_LEAVE)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_FOCUS_LOST, VGG_WINDOWEVENT_FOCUS_LOST)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_CLOSE, VGG_WINDOWEVENT_CLOSE)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_TAKE_FOCUS, VGG_WINDOWEVENT_TAKE_FOCUS)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_HIT_TEST, VGG_WINDOWEVENT_HIT_TEST)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_ICCPROF_CHANGED, VGG_WINDOWEVENT_ICCPROF_CHANGED)
  SWITCH_MAP_ITEM_DEF(SDL_WINDOWEVENT_DISPLAY_CHANGED, VGG_WINDOWEVENT_DISPLAY_CHANGED)
  SWITCH_MAP_ITEM_END(EWindowEventID::VGG_WINDOWEVENT_NONE)
}

inline EButtonState toEButtonState(int t)
{
  SWITCH_MAP_ITEM_BEGIN(t)
  SWITCH_MAP_ITEM_DEF(SDL_PRESSED, EButtonState::VGG_PRESSED)
  SWITCH_MAP_ITEM_DEF(SDL_RELEASED, EButtonState::VGG_RELEASE)
  SWITCH_MAP_ITEM_END(EButtonState::VGG_RELEASE)
}

inline VWindowEvent toVWindowEvent(const SDL_WindowEvent& e, float dpiScale)
{
  VWindowEvent v;
  v.timestamp = e.timestamp;
  v.windowID = e.windowID;
  v.event = toEWindowEventID((SDL_WindowEventID)e.event);
  v.data1 = e.data1;
  v.data2 = e.data2;
  v.drawableWidth = v.data1;
  v.drawableHeight = v.data2;
  if (v.event == VGG_WINDOWEVENT_RESIZED || v.event == VGG_WINDOWEVENT_SIZE_CHANGED)
  {
    v.drawableWidth = v.data1 * dpiScale;
    v.drawableHeight = v.data2 * dpiScale;
  }
  return v;
}

inline VKeysym toVKeysym(const SDL_Keysym& sym)
{
  VKeysym s;
  s.sym = (EVGGKeyCode)sym.sym;
  s.mod = (EVGGKeymod)sym.mod;
  return s;
}

inline VKeyboardEvent toVKeyboardEvent(const SDL_KeyboardEvent& e)
{
  VKeyboardEvent ve;
  ve.timestamp = e.timestamp;
  ve.state = toEButtonState(e.state);
  ve.repeat = e.repeat;
  ve.keysym = toVKeysym(e.keysym);
  return ve;
}

inline VTextEditingEvent toVTextEditingEvent(const SDL_TextEditingEvent& e)
{
  VTextEditingEvent v;
  v.timestamp = e.timestamp;
  v.start = e.start;
  v.length = e.length;
  std::strncpy(v.text, e.text, sizeof(v.text));
  return v;
}
inline VTextEditingExtEvent toVTextEditingExtEvent(const SDL_TextEditingExtEvent& e)
{
  VTextEditingExtEvent v;
  v.timestamp = e.timestamp;
  v.start = e.start;
  v.length = e.length;
  v.text = e.text;
  return v;
}
inline VTextInputEvent toVTextInputEvent(const SDL_TextInputEvent& e)
{
  VTextInputEvent v;
  v.timestamp = e.timestamp;
  std::strncpy(v.text, e.text, sizeof(v.text));
  return v;
}

inline VQuitEvent toVQuitEvent(const SDL_QuitEvent& e)
{
  VQuitEvent v;
  v.timestamp = e.timestamp;
  return v;
}

inline VDropEvent toVDropEvent(const SDL_DropEvent& e)
{
  VDropEvent v;
  v.timestamp = e.timestamp;
  v.file = e.file;
  return v;
}

inline VUserEvent toVUserEvent(const SDL_UserEvent& e)
{
  VUserEvent v;
  v.timestamp = e.timestamp;
  v.code = e.code;
  // We don't manage the memory
  v.data1 = e.data1;
  v.data2 = e.data2;
  return v;
}

inline VMouseMotionEvent toVMouseMotionEvent(const SDL_MouseMotionEvent& e, float dpiScale)
{
  VMouseMotionEvent v;
  v.timestamp = e.timestamp;
  v.windowY = e.y;
  v.windowX = e.x;
  v.canvasX = e.x * dpiScale;
  v.canvasY = e.y * dpiScale;
  v.yrel = e.yrel;
  v.xrel = e.xrel;
  v.canvasXRel = e.xrel * dpiScale;
  v.canvasYRel = e.yrel * dpiScale;
  v.state = toEButtonState(e.state);
  v.which = e.which;
  return v;
}

inline VMouseWheelEvent toVMouseWheelEvent(const SDL_MouseWheelEvent& e, float dpiScale)
{
  VMouseWheelEvent v;
  v.timestamp = e.timestamp;
  v.y = e.y;
  v.x = e.x;

  int mouseX, mouseY;
#ifdef EMSCRIPTEN
  SDL_GetMouseState(&mouseX, &mouseY);
#else
  mouseX = e.mouseX;
  mouseY = e.mouseY;
#endif

  v.mouseX = mouseX;
  v.mouseY = mouseY;
  v.canvasX = mouseX * dpiScale;
  v.canvasY = mouseY * dpiScale;
  v.preciseX = e.preciseX;
  v.preciseY = e.preciseY;
  v.direction = e.direction;
  return v;
}

inline VMouseButtonEvent toVMouseButtonEvent(const SDL_MouseButtonEvent& e, float dpiScale)
{
  VMouseButtonEvent v;
  v.timestamp = e.timestamp;
  v.windowY = e.y;
  v.windowX = e.x;
  v.canvasX = e.x * dpiScale;
  v.canvasY = e.y * dpiScale;
  v.state = toEButtonState(e.state);
  v.which = e.which;
  v.clicks = e.clicks;
  v.button = e.button;
  return v;
}

inline UEvent toUEvent(const SDL_Event& e, float dpiScale = 1.0)
{
  UEvent u;
  auto t = toEEventType((SDL_EventType)e.type);
  switch (e.type)
  {
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
      u.button = toVMouseButtonEvent(e.button, dpiScale);
      break;
    case SDL_MOUSEMOTION:
      u.motion = toVMouseMotionEvent(e.motion, dpiScale);
      break;
    case SDL_MOUSEWHEEL:
      u.wheel = toVMouseWheelEvent(e.wheel, dpiScale);
      break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      u.key = toVKeyboardEvent(e.key);
      break;
    case SDL_TEXTEDITING:
      u.edit = toVTextEditingEvent(e.edit);
      break;
    case SDL_TEXTEDITING_EXT:
      u.editExt = toVTextEditingExtEvent(e.editExt);
      break;
    case SDL_TEXTINPUT:
      u.text = toVTextInputEvent(e.text);
      break;
    case SDL_QUIT:
      u.quit = toVQuitEvent(e.quit);
      break;
    case SDL_DROPFILE:
    case SDL_DROPBEGIN:
    case SDL_DROPTEXT:
    case SDL_DROPCOMPLETE:
      u.drop = toVDropEvent(e.drop);
      break;
    case SDL_USEREVENT:
      u.user = toVUserEvent(e.user);
      break;
    case SDL_WINDOWEVENT:
      u.window = toVWindowEvent(e.window, dpiScale);
      break;
  }
  u.type = t;
  return u;
}

#undef SWITCH_MAP_ITEM_BEGIN
#undef SWITCH_MAP_ITEM_DEF
#undef SWITCH_MAP_ITEM_DEF_NULL
#undef SWITCH_MAP_ITEM_END
