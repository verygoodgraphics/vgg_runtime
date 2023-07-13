#pragma once

#include "UIEvent.hpp"
#include "Presenter/UIEvent.hpp"
#include "Utils/Utils.hpp"

#include <memory>

namespace VGG
{
namespace BrowserAdapter
{

class KeyboardEvent : public UIEvent
{
public:
  // getter
  std::string key() const
  {
    char key = event()->key;

    return { key };
  }

  bool repeat() const
  {
    return event()->repeat;
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
  std::shared_ptr<VGG::KeyboardEvent> event() const
  {
    auto result = getEvent<VGG::KeyboardEvent>();
    ASSERT(result);
    return result;
  }
};

} // namespace BrowserAdapter
} // namespace VGG