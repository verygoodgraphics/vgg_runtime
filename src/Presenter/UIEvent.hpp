#pragma once

#include <any>
#include <memory>
#include <string>

namespace VGG
{

// https://developer.mozilla.org/en-US/docs/Web/API/Element#events
enum class UIEventType
{
  invalid,

  // Keyboard events
  keydown,
  keyup,

  // Mouse events
  auxclick,
  click,
  contextmenu,
  dblclick,
  mousedown,
  mouseenter,
  mouseleave,
  mousemove,
  mouseout,
  mouseover,
  mouseup,

  // Touch events
  touchcancel,
  touchend,
  touchmove,
  touchstart
};

constexpr const char* ViewEventTypeToString(UIEventType e) noexcept
{
  switch (e)
  {
    case UIEventType::invalid:
      return "invalid";

    // Keyboard events
    case UIEventType::keydown:
      return "keydown";
    case UIEventType::keyup:
      return "keyup";

    // Mouse events
    case UIEventType::auxclick:
      return "auxclick";
    case UIEventType::click:
      return "click";
    case UIEventType::contextmenu:
      return "contextmenu";
    case UIEventType::dblclick:
      return "dblclick";
    case UIEventType::mousedown:
      return "mousedown";
    case UIEventType::mouseenter:
      return "mouseenter";
    case UIEventType::mouseleave:
      return "mouseleave";
    case UIEventType::mousemove:
      return "mousemove";
    case UIEventType::mouseout:
      return "mouseout";
    case UIEventType::mouseover:
      return "mouseover";
    case UIEventType::mouseup:
      return "mouseup";

    // Touch events
    case UIEventType::touchcancel:
      return "touchcancel";
    case UIEventType::touchend:
      return "touchend";
    case UIEventType::touchmove:
      return "touchmove";
    case UIEventType::touchstart:
      return "touchstart";

    default:
      throw std::invalid_argument("Unimplemented item");
  }
}

struct UIEvent
{
  const std::string path;
  UIEventType type;
  std::any data;
};
using UIEventPtr = std::shared_ptr<UIEvent>;

struct KeyboardEvent
{
};

struct MouseEvent
{
};

struct TouchEvent
{
};

} // namespace VGG