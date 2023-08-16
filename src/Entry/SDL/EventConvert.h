#pragma once

#include "Event/Event.h"
#include <SDL2/SDL_events.h>

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

inline UEvent toUEvent(const SDL_Event& e)
{
  UEvent u;
  u.type = toEEventType((SDL_EventType)e.type);
}

#undef SWITCH_MAP_ITEM_BEGIN
#undef SWITCH_MAP_ITEM_DEF
#undef SWITCH_MAP_ITEM_DEF_NULL
#undef SWITCH_MAP_ITEM_END
