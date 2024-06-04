#pragma once

#include "Application/Event/EventListener.hpp"
#include "viewer.hpp"

template<typename App>
class MyEventListener : public VGG::app::EventListener
{
public:
  bool onEvent(UEvent evt, void* userData) override
  {
    return viewer.onEvent(evt, userData);
  }
  Viewer viewer;
};
