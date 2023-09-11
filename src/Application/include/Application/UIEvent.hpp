#pragma once

#include "Domain/Event.hpp"
#include "EventVisitor.hpp"

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

  UIEvent(const PathType& path, UIEventType type)
    : m_path{ path }
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

  auto enumType()
  {
    return m_type;
  }

private:
  const PathType m_path;
  const UIEventType m_type;
};

using UIEventPtr = std::shared_ptr<UIEvent>;

struct KeyboardEvent
  : UIEvent
  , std::enable_shared_from_this<KeyboardEvent>
{
  const int key;

  const bool repeat;

  const bool altKey;
  const bool ctrlKey;
  const bool metaKey;
  const bool shiftKey;

  KeyboardEvent(const PathType& path,
                UIEventType type,
                int key,
                bool repeat = false,
                bool altKey = false,
                bool ctrlKey = false,
                bool metaKey = false,
                bool shiftKey = false)
    : UIEvent(path, type)
    , key{ key }
    , repeat{ repeat }
    , altKey{ altKey }
    , ctrlKey{ ctrlKey }
    , metaKey{ metaKey }
    , shiftKey{ shiftKey }
  {
    assert(type == UIEventType::keydown || type == UIEventType::keyup);
  }

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

struct MouseEvent
  : UIEvent
  , std::enable_shared_from_this<MouseEvent>
{
  const int button;

  const int x;
  const int y;

  const int movementX;
  const int movementY;

  const bool altKey;
  const bool ctrlKey;
  const bool metaKey;
  const bool shiftKey;

  MouseEvent(const PathType& path,
             UIEventType type,
             int button = 0,
             int x = 0,
             int y = 0,
             int movementX = 0,
             int movementY = 0,
             bool altKey = false,
             bool ctrlKey = false,
             bool metaKey = false,
             bool shiftKey = false)
    : UIEvent(path, type)
    , button{ button }
    , x{ x }
    , y{ y }
    , movementX{ movementX }
    , movementY{ movementY }
    , altKey{ altKey }
    , ctrlKey{ ctrlKey }
    , metaKey{ metaKey }
    , shiftKey{ shiftKey }
  {
    assert(type >= UIEventType::auxclick && type <= UIEventType::mouseup);
  }

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

struct TouchEvent
  : UIEvent
  , std::enable_shared_from_this<TouchEvent>
{
  TouchEvent(const PathType& path, UIEventType type)
    : UIEvent(path, type)
  {
    assert(type >= UIEventType::touchcancel && type <= UIEventType::touchstart);
  }

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

} // namespace VGG
