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

public:
  EventStore(EventPtr event)
    : m_event{ event }
  {
  }
  std::string eventId()
  {
    m_event->accept(this);
    return {}; // todo
  }

  virtual void visit(KeyboardEvent*)
  {
    UIEvent<KeyboardEvent>::store(std::dynamic_pointer_cast<KeyboardEvent>(m_event));
  }

  virtual void visit(MouseEvent*)
  {
    UIEvent<MouseEvent>::store(std::dynamic_pointer_cast<MouseEvent>(m_event));
  }

  virtual void visit(TouchEvent*)
  {
    UIEvent<VGG::UIEvent>::store(std::dynamic_pointer_cast<VGG::UIEvent>(m_event));
  }
};

} // namespace NodeAdapter
} // namespace VGG