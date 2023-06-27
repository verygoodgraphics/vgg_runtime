#pragma once

#include <any>
#include <cassert>
#include <memory>
#include <string>

namespace VGG
{

struct Event
{
  virtual ~Event() = default;

  virtual void preventDefault()
  {
  }
};

using EventPtr = std::shared_ptr<Event>;

} // namespace VGG