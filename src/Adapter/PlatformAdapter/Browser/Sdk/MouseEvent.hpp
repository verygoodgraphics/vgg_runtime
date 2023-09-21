#pragma once

#include "UIEvent.hpp"
#include "Utility/Log.hpp"

#include <memory>

namespace VGG
{
namespace BrowserAdapter
{

class MouseEvent : public UIEvent
{
public:
  // getter
  int button() const
  {
    return event()->button;
  }

  int x() const
  {
    return event()->x;
  }
  int y() const
  {
    return event()->y;
  }

  int movementX() const
  {
    return event()->movementX;
  }
  int movementY() const
  {
    return event()->movementY;
  }

  bool altkey() const
  {
    return event()->altKey;
  }
  bool ctrlkey() const
  {
    return event()->ctrlKey;
  }
  bool metakey() const
  {
    return event()->metaKey;
  }
  bool shiftkey() const
  {
    return event()->shiftKey;
  }

  // methods

private:
  std::shared_ptr<VGG::MouseEvent> event() const
  {
    auto result = getEvent<VGG::MouseEvent>();
    ASSERT(result);
    return result;
  }
};

} // namespace BrowserAdapter
} // namespace VGG
