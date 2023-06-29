#pragma once

#include "Event.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <stdexcept>

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

class UIEvent : public Event
{
public:
  using PathType = std::string;

  UIEvent(const PathType& path, const UIEventType type)
    : m_path{ path }
    , m_type{ type }
  {
  }

  UIEvent(PathType&& path, UIEventType type)
    : m_path{ std::move(path) }
    , m_type{ type }
  {
  }

  virtual ~UIEvent() = default;

  PathType path()
  {
    return m_path;
  }

  virtual std::string target()
  {
    return m_path;
  }

  virtual std::string type()
  {
    return UIEventTypeToString(m_type);
  }

private:
  const PathType m_path;
  const UIEventType m_type;
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

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

struct MouseEvent : UIEvent
{
  const int button;

  MouseEvent(const PathType& path, UIEventType type, int button = 0)
    : UIEvent(path, type)
    , button{ button }
  {
    assert(type >= UIEventType::auxclick && type <= UIEventType::mouseup);
  }

  MouseEvent(PathType&& path, UIEventType type, int button = 0)
    : UIEvent(std::move(path), type)
    , button{ button }
  {
    assert(type >= UIEventType::auxclick && type <= UIEventType::mouseup);
  }

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
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

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

} // namespace VGG
