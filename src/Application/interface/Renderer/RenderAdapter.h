#pragma once
#include "Event/EventListener.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/VGGLayer.h"
#include "Scene/Scene.h"
#include <queue>

namespace VGG
{

class ZoomerListener
  : public Zoomer
  , public EventListener
{
  bool m_panning{ false };

public:
  bool dispatchEvent(UEvent e, void* userData) override
  {
    // if (!m_panning && e.type == VGG_MOUSEBUTTONDOWN &&
    //     (SDL_GetKeyboardState(nullptr)[VGG_SCANCODE_SPACE]))
    // {
    //   m_panning = true;
    //   return true;
    // }
    // else if (m_panning && e.type == VGG_MOUSEBUTTONUP)
    // {
    //   m_panning = false;
    //   return true;
    // }
    // else if (m_panning && e.type == VGG_MOUSEMOTION)
    // {
    //   doTranslate(e.motion.xrel, e.motion.yrel);
    //   return true;
    // }
    // else if (e.type == VGG_MOUSEWHEEL && (SDL_GetModState() & VGG_KMOD_CTRL))
    // {
    //   int mx, my;
    //   SDL_GetMouseState(&mx, &my);
    //   doZoom((e.wheel.y > 0 ? 1.0 : -1.0) * 0.03, mx, my);
    //   return true;
    // }
    return false;
  }
};

class SceneEventListener
  : public Scene
  , public EventListener
{
  bool m_panning{ false };

public:
  bool dispatchEvent(UEvent e, void* userData) override
  {
    if (zoomer())
    {
      auto z = zoomer();

      // if (!m_panning && e.type == VGG_MOUSEBUTTONDOWN &&
      //     (SDL_GetKeyboardState(nullptr)[VGG_SCANCODE_SPACE]))
      // {
      //   m_panning = true;
      //   return true;
      // }
      // else if (m_panning && e.type == VGG_MOUSEBUTTONUP)
      // {
      //   m_panning = false;
      //   return true;
      // }
      // else if (m_panning && e.type == VGG_MOUSEMOTION)
      // {
      //   z->doTranslate(e.motion.xrel, e.motion.yrel);
      //   return true;
      // }
      // else if (e.type == VGG_MOUSEWHEEL && (SDL_GetModState() & VGG_KMOD_CTRL))
      // {
      //   int mx, my;
      //   SDL_GetMouseState(&mx, &my);
      //   z->doZoom((e.wheel.y > 0 ? 1.0 : -1.0) * 0.03, mx, my);
      //   return true;
      // }
    }
    return false;
  }
};

class LayerEventAdapter__pImpl;
class LayerEventAdapter
{
  VGG_DECL_IMPL(LayerEventAdapter)
public:
  LayerEventAdapter() = default;
  LayerEventAdapter(std::shared_ptr<VLayer> layer);
  void postEvent(UEvent e);
  void sendEvent(UEvent e);
  void addSceneListener(std::shared_ptr<SceneEventListener> listener);

  void beginFrame();
  void render();
  void endFrame();

  ~LayerEventAdapter();

protected:
  bool onEvent(UEvent e);
};

} // namespace VGG
