#pragma once

#include "Event.h"
#include <memory>
namespace VGG
{
class EventListener : public std::enable_shared_from_this<EventListener>
{
public:
  virtual bool dispatchEvent(UEvent e, void* userData) = 0;
};
} // namespace VGG

// general EventListener
