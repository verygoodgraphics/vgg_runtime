#include "MouseEventWrapper.hpp"

namespace VGG
{

int MouseEventWrapper::button() const
{
  return 0; // todo
}

void MouseEventWrapper::bindCppEvent()
{
  m_event = s_event;
}

void MouseEventWrapper::preventDefault()
{
  m_event->preventDefault();
}

} // namespace VGG