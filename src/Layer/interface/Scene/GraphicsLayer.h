#pragma once
#include <memory>
#include <optional>
#include "Core/Node.h"
#include "../../../Application/include/Event/EventListener.h"
#include "Zoomer.h"

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
    if (!m_panning && e.type == VGG_MOUSEBUTTONDOWN &&
        (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_SPACE]))
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
    else if (e.type == VGG_MOUSEWHEEL && (SDL_GetModState() & KMOD_CTRL))
    {
      int mx, my;
      SDL_GetMouseState(&mx, &my);
      doZoom((e.wheel.y > 0 ? 1.0 : -1.0) * 0.03, mx, my);
      return true;
    }
    return false;
  }
};

// This listner has renderable capability
class RenderEventListener : public EventListener
{
public:
  virtual bool onPaintEvent(VPaintEvent e) = 0;
};

enum class EGraphicAPI
{
  GA_OPENGL
};

struct LayerConfig
{
  EGraphicAPI graphicsAPI{ EGraphicAPI::GA_OPENGL };
  float drawableSize[2];
  float dpi{ 1.0 };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

enum class ELayerError
{
  TextureSizeOutOfRangeError,
  EGLNoDisplayError,
  EGLGetAttribError,
  MakeCurrentContextError,
  HasInitError,
  RenderEngineError,
  UnknownError,
};

class Graphics__pImpl;
class Graphics : public std::enable_shared_from_this<Graphics>
{
  VGG_DECL_IMPL(Graphics)
public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) = 0;
  // event will not be handled until beginFrame
  void postEvent(UEvent e);
  // event will be handled immediately
  void sendEvent(UEvent e);
  void addEventListener(std::shared_ptr<EventListener> listener);
  void addRenderListener(std::shared_ptr<RenderEventListener> listener);
  virtual void beginFrame();
  // render is just a send special event
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
  virtual ~Graphics() = 0;

protected:
  Graphics();
  virtual bool onEvent(UEvent e) = 0;
  void sendRenderEvent(VPaintEvent e);
};

} // namespace VGG
