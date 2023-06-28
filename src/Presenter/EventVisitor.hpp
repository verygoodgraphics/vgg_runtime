#pragma once

namespace VGG
{

struct KeyboardEvent;
struct MouseEvent;
struct TouchEvent;

class EventVisitor
{
public:
  virtual void visit(KeyboardEvent* e) = 0;
  virtual void visit(MouseEvent* e) = 0;
  virtual void visit(TouchEvent* e) = 0;
};

} // namespace VGG
