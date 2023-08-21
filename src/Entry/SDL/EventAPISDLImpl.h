#pragma once

#include "Entry/SDL/EventConvert.h"
#include "Event/EventAPI.h"
#include "Event/Keycode.h"
#include <SDL2/SDL_keyboard.h>
#include <cstdint>

class EventAPISDLImpl : public EventAPI
{
public:
  EVGGKeymod getModState() override
  {
    return EVGGKeymod(SDL_GetModState());
  }
  uint8_t* getKeyboardState(int* nums) override
  {
    return (uint8_t*)SDL_GetKeyboardState(nums);
  }
};
