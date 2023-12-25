/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

constexpr const char* uiEventTypeToString(EUIEventType e)
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
  using TargetIdType = std::string;
  using TargetPathType = std::string;

private:
  const EUIEventType   m_type;
  const TargetIdType   m_targetId;
  const TargetPathType m_targetPath;

public:
  UIEvent(EUIEventType type, const TargetIdType& targetId, const TargetPathType& targetPath)
    : m_type{ type }
    , m_targetId{ targetId }
    , m_targetPath{ targetPath }
  {
  }

  virtual ~UIEvent() = default;

  TargetIdType targetId()
  {
    return m_targetId;
  }
  TargetPathType targetPath()
  {
    return m_targetPath;
  }

  virtual std::string target()
  {
    return targetId();
  }

  virtual std::string type()
  {
    return uiEventTypeToString(m_type);
  }

  auto enumType()
  {
    return m_type;
  }
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

  KeyboardEvent(
    EUIEventType          type,
    const TargetIdType&   targetId,
    const TargetPathType& targetPath,
    int                   key,
    bool                  repeat = false,
    bool                  altKey = false,
    bool                  ctrlKey = false,
    bool                  metaKey = false,
    bool                  shiftKey = false)
    : UIEvent(type, targetId, targetPath)
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

  MouseEvent(
    EUIEventType          type,
    const TargetIdType&   targetId,
    const TargetPathType& targetPath,
    int                   button = 0,
    int                   x = 0,
    int                   y = 0,
    int                   movementX = 0,
    int                   movementY = 0,
    bool                  altKey = false,
    bool                  ctrlKey = false,
    bool                  metaKey = false,
    bool                  shiftKey = false)
    : UIEvent(type, targetId, targetPath)
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
  TouchEvent(EUIEventType type, const TargetIdType& targetId, const TargetPathType& targetPath)
    : UIEvent(type, targetId, targetPath)
  {
    assert(type >= EUIEventType::TOUCHCANCEL && type <= EUIEventType::TOUCHSTART);
  }

  void accept(EventVisitor* visitor) override
  {
    visitor->visit(this);
  }
};

} // namespace VGG
