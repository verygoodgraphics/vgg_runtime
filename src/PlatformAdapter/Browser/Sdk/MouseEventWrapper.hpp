#pragma once

#include "Presenter/UIEvent.hpp"

#include <memory>

namespace VGG
{

class MouseEventWrapper
{
  inline static std::shared_ptr<Event> s_event;
  std::shared_ptr<Event> m_event;

public:
  // vgg
  static void store(std::shared_ptr<Event> event)
  {
    s_event = event;
  }
  void bindCppEvent();

  // getter
  int button() const;

  // methods
  void preventDefault();
};

} // namespace VGG