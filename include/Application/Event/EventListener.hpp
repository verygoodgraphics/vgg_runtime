#pragma once

#include "Event.hpp"
#include <memory>
namespace VGG::app
{
class EventListener : public std::enable_shared_from_this<EventListener>
{
public:
  virtual bool onEvent(UEvent e, void* userData) = 0;
  virtual ~EventListener() = default;
};
} // namespace VGG::app

// general EventListener
