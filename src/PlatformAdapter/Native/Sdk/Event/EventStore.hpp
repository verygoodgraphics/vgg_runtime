#pragma once

#include "Presenter/EventVisitor.hpp"
#include "Presenter/UIEvent.hpp"
#include "UIEvent.hpp"

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

  virtual void visit(KeyboardEvent*)
  {
    m_event_id = UIEvent<KeyboardEvent>::store(std::dynamic_pointer_cast<KeyboardEvent>(m_event));
  }

  virtual void visit(MouseEvent*)
  {
    m_event_id = UIEvent<MouseEvent>::store(std::dynamic_pointer_cast<MouseEvent>(m_event));
  }

  virtual void visit(TouchEvent*)
  {
    m_event_id = UIEvent<TouchEvent>::store(std::dynamic_pointer_cast<TouchEvent>(m_event));
  }
};

} // namespace NodeAdapter
} // namespace VGG