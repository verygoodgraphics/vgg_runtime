#pragma once

#include "EventConvert.hpp"
#include "Application/Event/EventAPI.hpp"
#include "Application/Event/Keycode.hpp"

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
