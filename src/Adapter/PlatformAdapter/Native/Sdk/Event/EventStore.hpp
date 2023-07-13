#pragma once

#include "Application/EventVisitor.hpp"
#include "UIEvent.hpp"
#include "MouseEvent.hpp"
#include "KeyboardEvent.hpp"
#include "TouchEvent.hpp"
#include "Utils/Utils.hpp"

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