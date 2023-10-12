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

#include "Application/EventVisitor.hpp"
#include "UIEvent.hpp"
#include "MouseEvent.hpp"
#include "KeyboardEvent.hpp"
#include "TouchEvent.hpp"
#include "Utility/Log.hpp"

namespace VGG
{
namespace NodeAdapter
{

class EventStore : public EventVisitor
{
  EventPtr m_event;
  int m_event_id;

public:
  EventStore(EventPtr event)
    : m_event{ event }
  {
    ASSERT(m_event);
  }
  std::string eventId()
  {
    m_event->accept(this);
    return std::to_string(m_event_id);
  }

  virtual void visit(VGG::KeyboardEvent* e) override
  {
    m_event_id = KeyboardEvent::store(e->shared_from_this());
  }

  virtual void visit(VGG::MouseEvent* e) override
  {
    m_event_id = MouseEvent::store(e->shared_from_this());
  }

  virtual void visit(VGG::TouchEvent* e) override
  {
    m_event_id = TouchEvent::store(e->shared_from_this());
  }
};

} // namespace NodeAdapter
} // namespace VGG
