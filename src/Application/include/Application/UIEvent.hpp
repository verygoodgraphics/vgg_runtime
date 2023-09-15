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
enum class EUIEventType
{
  // Keyboard events
  KEYDOWN,
  KEYUP,

  // Mouse events
  AUXCLICK,
  CLICK,
  CONTEXTMENU,
  DBLCLICK,
  MOUSEDOWN,
  MOUSEENTER,
  MOUSELEAVE,
  MOUSEMOVE,
  MOUSEOUT,
  MOUSEOVER,
  MOUSEUP,

  // Touch events
  TOUCHCANCEL,
  TOUCHEND,
  TOUCHMOVE,
  TOUCHSTART
};

constexpr const char* uiEventTypeToString(EUIEventType e) noexcept
{
  switch (e)
  {
    // Keyboard events
    case EUIEventType::KEYDOWN:
      return "keydown";
    case EUIEventType::KEYUP:
      return "keyup";

    // Mouse events
    case EUIEventType::AUXCLICK:
      return "auxclick";
    case EUIEventType::CLICK:
      return "click";
    case EUIEventType::CONTEXTMENU:
      return "contextmenu";
    case EUIEventType::DBLCLICK:
      return "dblclick";
    case EUIEventType::MOUSEDOWN:
      return "mousedown";
    case EUIEventType::MOUSEENTER:
      return "mouseenter";
    case EUIEventType::MOUSELEAVE:
      return "mouseleave";
    case EUIEventType::MOUSEMOVE:
      return "mousemove";
    case EUIEventType::MOUSEOUT:
      return "mouseout";
    case EUIEventType::MOUSEOVER:
      return "mouseover";
    case EUIEventType::MOUSEUP:
      return "mouseup";

    // Touch events
    case EUIEventType::TOUCHCANCEL:
      return "touchcancel";
    case EUIEventType::TOUCHEND:
      return "touchend";
    case EUIEventType::TOUCHMOVE:
      return "touchmove";
    case EUIEventType::TOUCHSTART:
      return "touchstart";

    default:
      throw std::invalid_argument("Unimplemented item");
  }
}

class UIEvent : public Event
{
public:
  using PathType = std::string;

  UIEvent(const PathType& path, EUIEventType type)
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
    return uiEventTypeToString(m_type);
  }

  auto enumType()
  {
    return m_type;
  }

private:
  const PathType m_path;
  const EUIEventType m_type;
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
                EUIEventType type,
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
    assert(type == EUIEventType::KEYDOWN || type == EUIEventType::KEYUP);
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
             EUIEventType type,
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
    assert(type >= EUIEventType::AUXCLICK && type <= EUIEventType::MOUSEUP);
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
  TouchEvent(const PathType& path, EUIEventType type)
    : UIEvent(path, type)
  {
    assert(type >= EUIEventType::TOUCHCANCEL && type <= EUIEventType::TOUCHSTART);
  }

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

} // namespace VGG
