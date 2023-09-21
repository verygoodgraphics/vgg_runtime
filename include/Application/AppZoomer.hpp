#pragma once

#include "Event/EventListener.hpp"
#include "Event/EventAPI.hpp"

#include "VGG/Layer/Zoomer.hpp"

namespace VGG::app
{

class AppZoomer
  : public Zoomer
  , public EventListener
{
  bool m_panning{ false };

public:
  bool onEvent(UEvent e, void* userData) override
  {
    if (!m_panning && e.type == VGG_MOUSEBUTTONDOWN &&
        (EventManager::getKeyboardState(nullptr)[VGG_SCANCODE_SPACE]))
    {
      m_panning = true;
      return true;
    }
    else if (m_panning && e.type == VGG_MOUSEBUTTONUP)
    {
      m_panning = false;
      return true;
    }
    else if (m_panning && e.type == VGG_MOUSEMOTION)
    {
      doTranslate(e.motion.xrel, e.motion.yrel);
      return true;
    }
    else if (e.type == VGG_MOUSEWHEEL && (EventManager::getModState() & VGG_KMOD_CTRL))
    {
      int mx = e.wheel.mouseX;
      int my = e.wheel.mouseY;
      doZoom((e.wheel.y > 0 ? 1.0 : -1.0) * 0.03, mx, my);
      return true;
    }
    return false;
  }
};
} // namespace VGG::app
