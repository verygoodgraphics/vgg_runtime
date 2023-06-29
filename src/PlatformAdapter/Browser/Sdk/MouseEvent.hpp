#pragma once

#include "UIEvent.hpp"
#include "Presenter/UIEvent.hpp"

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
    return getEvent<VGG::MouseEvent>()->button;
  }

  // methods
};

} // namespace BrowserAdapter
} // namespace VGG