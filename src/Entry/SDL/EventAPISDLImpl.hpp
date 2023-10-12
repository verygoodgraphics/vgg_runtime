/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "EventConvert.hpp"
#include "Application/Event/EventAPI.hpp"
#include "Application/Event/Keycode.hpp"

#include <SDL_keyboard.h>

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
