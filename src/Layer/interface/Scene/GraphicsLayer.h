#pragma once
#include <memory>
#include <optional>
#include "Core/Node.h"
#include "../../../Application/include/Event/EventListener.h"
#include "Zoomer.h"

namespace VGG
{


// This listner has renderable capability
class RenderEventListener : public EventListener
{
public:
  virtual bool onPaintEvent(VPaintEvent e) = 0;
};

class RenderContext
{
public:
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
