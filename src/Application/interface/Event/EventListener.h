#pragma once

#include "Event.h"
#include <memory>
namespace VGG
{
class EventListener : public std::enable_shared_from_this<EventListener>
{
public:
  virtual bool onEvent(UEvent e, void* userData) = 0;
  virtual ~EventListener() = default;
};
} // namespace VGG

// general EventListener
