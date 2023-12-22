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

#include "Keycode.hpp"
#include "Scancode.hpp"

#include <memory>

// These API style is derived from SDL, and the usage is keep same with SDL's currently.
// Refer to SDL_keyboard.h for more detail.
// Only few APIs are implemented now, they will be enriched when needed.
class EventAPI : public std::enable_shared_from_this<EventAPI>
{
public:
  EventAPI() = default;
  EventAPI(const EventAPI&) = delete;
  EventAPI(EventAPI&&) = delete;
  EventAPI&          operator=(const EventAPI&) = delete;
  EventAPI&          operator=(EventAPI&&) = delete;
  virtual EVGGKeymod getModState() = 0;
  virtual uint8_t*   getKeyboardState(int* nums) = 0;
  virtual ~EventAPI() = default;
};

class EventManager
{
public:
  EventManager() = delete;
  EventManager(const EventAPI&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;

  static void        registerEventAPI(std::unique_ptr<EventAPI> impl);
  static EVGGKeymod  getModState();
  static uint8_t*    getKeyboardState(int* nums);
  static EVGGKeyCode getKeyFromScancode(EVGGScancode scancode);
};
