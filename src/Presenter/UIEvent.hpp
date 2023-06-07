#pragma once

#include <any>
#include <cassert>
#include <memory>
#include <string>

namespace VGG
{

// https://developer.mozilla.org/en-US/docs/Web/API/Element#events
enum class UIEventType
{
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

constexpr const char* UIEventTypeToString(UIEventType e) noexcept
{
  switch (e)
  {
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
  using PathType = std::string;

  const PathType path;
  const UIEventType type;

  UIEvent(const PathType& path, const UIEventType type)
    : path{ path }
    , type{ type }
  {
  }

  UIEvent(PathType&& path, UIEventType type)
    : path{ std::move(path) }
    , type{ type }
  {
  }

  virtual ~UIEvent() = default;
};

using UIEventPtr = std::shared_ptr<UIEvent>;

struct KeyboardEvent : UIEvent
{
  KeyboardEvent(const PathType& path, UIEventType type)
    : UIEvent(path, type)
  {
    assert(type == UIEventType::keydown || type == UIEventType::keyup);
  }

  KeyboardEvent(PathType&& path, UIEventType type)
    : UIEvent(std::move(path), type)
  {
    assert(type == UIEventType::keydown || type == UIEventType::keyup);
  }
};

struct MouseEvent : UIEvent
{
  MouseEvent(const PathType& path, UIEventType type)
    : UIEvent(path, type)
  {
    assert(type >= UIEventType::auxclick && type <= UIEventType::mouseup);
  }

  MouseEvent(PathType&& path, UIEventType type)
    : UIEvent(std::move(path), type)
  {
    assert(type >= UIEventType::auxclick && type <= UIEventType::mouseup);
  }
};

struct TouchEvent : UIEvent
{
  TouchEvent(const PathType& path, UIEventType type)
    : UIEvent(path, type)
  {
    assert(type >= UIEventType::touchcancel && type <= UIEventType::touchstart);
  }

  TouchEvent(PathType&& path, UIEventType type)
    : UIEvent(std::move(path), type)
  {
    assert(type >= UIEventType::touchcancel && type <= UIEventType::touchstart);
  }
};

} // namespace VGG