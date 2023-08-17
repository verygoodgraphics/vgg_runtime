#pragma once

#include "Core/Node.h"
#include "../../../Application/include/Event/Event.h"
#include <memory>

class EventListener : std::enable_shared_from_this<EventListener>
{
public:
  virtual void dispatchEvent(UEvent e) = 0;
};
