#pragma once

#include "EventVisitor.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

namespace VGG
{

struct Event
{
  virtual ~Event() = default;

  // Getter
  virtual std::string target() = 0;
  virtual std::string type() = 0;

  // Method
  virtual void accept(EventVisitor* visitor) = 0;
  virtual void preventDefault()
  {
    std::cout << "Event::preventDefault called" << std::endl;
  }
};

using EventPtr = std::shared_ptr<Event>;

} // namespace VGG