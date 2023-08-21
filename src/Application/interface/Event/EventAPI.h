#pragma once
#include <memory>

class EventAPI
{
public:
  EventAPI(const EventAPI&) = delete;
  EventAPI(EventAPI&&) = delete;
  EventAPI& operator=(const EventAPI&) = delete;
  EventAPI& operator=(EventAPI&&) = delete;

  virtual void getModStateImpl() = 0;
  virtual void getKeyboardStateImpl() = 0;
  virtual ~EventAPI() = default;
};

class EventManager
{
public:
  static void registerEventAPI(std::unique_ptr<EventAPI> impl);
  static void getModState();
};
