#pragma once
#include "Application/interface/Event/EventListener.h"
#include "Application/interface/Event/EventAPI.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/VGGLayer.h"
#include "Scene/Scene.h"
#include <queue>

namespace VGG
{
class VLayer;

class EventListenerScene
  : public Scene
  , public EventListener
{
  bool m_panning{ false };

public:
  bool onEvent(UEvent e, void* userData) override
  {
    if (zoomer())
    {
      auto z = zoomer();

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
        z->doTranslate(e.motion.xrel, e.motion.yrel);
        return true;
      }
      else if (e.type == VGG_MOUSEWHEEL && (EventManager::getModState() & VGG_KMOD_CTRL))
      {
        int mx = e.wheel.preciseX;
        int my = e.wheel.preciseY;
        // SDL_GetMouseState(&mx, &my);
        z->doZoom((e.wheel.y > 0 ? 1.0 : -1.0) * 0.03, mx, my);
        return true;
      }
    }
    return false;
  }
};

class EventDispatcherLayer__pImpl;
class EventDispatcherLayer
{
  VGG_DECL_IMPL(EventDispatcherLayer)
public:
  EventDispatcherLayer() = default;
  EventDispatcherLayer(std::shared_ptr<VLayer> layer);
  void postEvent(UEvent e, void* userData);
  void sendEvent(UEvent e, void* userData);
  void addSceneListener(std::shared_ptr<EventListenerScene> listener);

  void beginFrame();
  void render();
  void endFrame();

  ~EventDispatcherLayer();

protected:
  bool onEvent(UEvent e);
};

} // namespace VGG
