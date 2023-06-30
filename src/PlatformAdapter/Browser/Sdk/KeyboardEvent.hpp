#pragma once

#include "UIEvent.hpp"
#include "Presenter/UIEvent.hpp"

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

  // methods

private:
  std::shared_ptr<VGG::KeyboardEvent> event() const
  {
    return getEvent<VGG::KeyboardEvent>();
  }
};

} // namespace BrowserAdapter
} // namespace VGG