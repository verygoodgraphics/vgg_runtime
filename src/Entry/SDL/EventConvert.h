#pragma once

#include "Event/Event.h"
#include <SDL2/SDL_events.h>
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
  SWITCH_MAP_ITEM_DEF(SDL_QUIT, VGG_QUIT)
  SWITCH_MAP_ITEM_END(EEventType::VGG_USEREVENT)
}

inline EButtonState toEButtonState(int t)
{
  SWITCH_MAP_ITEM_BEGIN(t)
  SWITCH_MAP_ITEM_DEF(SDL_PRESSED, EButtonState::VGG_Pressed)
  SWITCH_MAP_ITEM_DEF(SDL_RELEASED, EButtonState::VGG_Release)
  SWITCH_MAP_ITEM_END(EButtonState::VGG_Release)
}

inline VKeyboardEvent toVKeyboardEvent(const SDL_KeyboardEvent& e)
{
  VKeyboardEvent ve;
  ve.type = toEEventType((SDL_EventType)e.type);
  ve.timestamp = e.timestamp;
  ve.state = toEButtonState(e.state);
  ve.repeat = e.repeat;
  return ve;
}

inline VTextEditingEvent toVTextEditingEvent(const SDL_TextEditingEvent& e)
{
  VTextEditingEvent v;
  v.timestamp = e.timestamp;
  v.start = e.start;
  v.length = e.length;
  std::strcpy(v.text, e.text);
  return v;
}
inline VTextEditingExtEvent toVTextEditingExtEvent(const SDL_TextEditingExtEvent& e)
{
  VTextEditingExtEvent v;
  v.timestamp = e.timestamp;
  v.start = e.start;
  v.length = e.length;
  std::strcpy(v.text, e.text);
  return v;
}
inline VTextInputEvent toVTextInputEvent(const SDL_TextInputEvent& e)
{
  VTextInputEvent v;
  v.timestamp = e.timestamp;
  std::strcpy(v.text, e.text);
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

inline VMouseMotionEvent toVMouseMotionEvent(const SDL_MouseMotionEvent& e)
{
  VMouseMotionEvent v;
  v.timestamp = e.timestamp;
  v.y = e.y;
  v.x = e.x;
  v.yrel = e.yrel;
  v.xrel = e.xrel;
  v.state = toEButtonState(e.state);
  v.which = e.which;
  return v;
}

inline VMouseWheelEvent toVMouseWheelEvent(const SDL_MouseWheelEvent& e)
{
  VMouseWheelEvent v;
  v.timestamp = e.timestamp;
  v.y = e.y;
  v.x = e.x;
  v.mouseX = e.mouseX;
  v.mouseY = e.mouseY;
  v.preciseX = e.preciseX;
  v.preciseY = e.preciseY;
  v.direction = e.direction;
  return v;
}

inline VMouseButtonEvent toVMouseButtonEvent(const SDL_MouseButtonEvent& e)
{
  VMouseButtonEvent v;
  v.timestamp = e.timestamp;
  v.y = e.y;
  v.x = e.x;
  v.state = toEButtonState(e.state);
  v.which = e.which;
  v.clicks = e.clicks;
  v.button = e.button;
  return v;
}

inline UEvent toUEvent(const SDL_Event& e)
{
  UEvent u;
  u.type = toEEventType((SDL_EventType)e.type);
  switch (e.type)
  {
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
      u.button = toVMouseButtonEvent(e.button);
      break;
    case SDL_MOUSEMOTION:
      u.motion = toVMouseMotionEvent(e.motion);
      break;
    case SDL_MOUSEWHEEL:
      u.wheel = toVMouseWheelEvent(e.wheel);
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
  }
  return u;
}

#undef SWITCH_MAP_ITEM_BEGIN
#undef SWITCH_MAP_ITEM_DEF
#undef SWITCH_MAP_ITEM_DEF_NULL
#undef SWITCH_MAP_ITEM_END
