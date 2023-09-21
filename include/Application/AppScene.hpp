#pragma once

#include "AppZoomer.hpp"
#include "Event/EventListener.hpp"

#include "VGG/Layer/Scene.hpp"

namespace VGG::app
{

class AppScene
  : public Scene
  , public EventListener
{
protected:
  std::shared_ptr<AppZoomer> m_zoomerListener;

public:
  // By setting a zoomer listener to customize your own operation
  void setZoomerListener(std::shared_ptr<AppZoomer> zoomerListener)
  {
    m_zoomerListener = std::move(zoomerListener);
    setZoomer(m_zoomerListener);
  }

  bool onEvent(UEvent e, void* userData) override
  {
    if (m_zoomerListener)
      return m_zoomerListener->onEvent(e, userData);
    return false;
  }
};
} // namespace VGG::app
