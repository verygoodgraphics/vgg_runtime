#pragma once
#include "Event/Keycode.h"
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
  EventAPI& operator=(const EventAPI&) = delete;
  EventAPI& operator=(EventAPI&&) = delete;
  virtual EVGGKeymod getModState() = 0;
  virtual uint8_t* getKeyboardState(int* nums) = 0;
  virtual ~EventAPI() = default;
};

class EventManager
{
public:
  static void registerEventAPI(std::unique_ptr<EventAPI> impl);
  static EVGGKeymod getModState();
  static uint8_t* getKeyboardState(int* nums);
};
