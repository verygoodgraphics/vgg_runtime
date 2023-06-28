#pragma once

#include "Presenter/EventVisitor.hpp"
#include "Presenter/UIEvent.hpp"
#include "MouseEvent.hpp"
#include "KeyboardEvent.hpp"
// #include "TouchEvent.hpp"

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
  }
  std::string eventId()
  {
    m_event->accept(this);
    return std::to_string(m_event_id);
  }

  virtual void visit(VGG::KeyboardEvent*) override
  {
    m_event_id = KeyboardEvent::store(std::dynamic_pointer_cast<VGG::KeyboardEvent>(m_event));
  }

  virtual void visit(VGG::MouseEvent*) override
  {
    m_event_id = MouseEvent::store(std::dynamic_pointer_cast<VGG::MouseEvent>(m_event));
  }

  virtual void visit(VGG::TouchEvent*) override
  {
    // m_event_id = TouchEvent::store(std::dynamic_pointer_cast<VGG::TouchEvent>(m_event));
  }
};

} // namespace NodeAdapter
} // namespace VGG