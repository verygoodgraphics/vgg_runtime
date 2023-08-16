#pragma once
#include <memory>
#include <optional>

#include "Core/Node.h"

#include "../../../Application/include/Event/Event.h"

namespace VGG
{

enum class EGraphicAPI
{
  GA_OPENGL
};

class Scene;

struct LayerConfig
{
  EGraphicAPI graphicsAPI{ EGraphicAPI::GA_OPENGL };
  float dpiScale[2];
};

struct LayerEvent
{
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
class GraphicsEventListener__pImpl;
class GraphicsEventListener : std::enable_shared_from_this<GraphicsEventListener>
{
  VGG_DECL_IMPL(GraphicsEventListener);

public:
  GraphicsEventListener();
  virtual void dispatchEvent(UEvent e, void* userData);
};

class Graphics : public std::enable_shared_from_this<Graphics>
{
public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) = 0;
  void pushEvent(UEvent e);
  void setEventListener(std::shared_ptr<GraphicsEventListener> listener);
  void setScene(std::shared_ptr<Scene> scene);
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
  virtual ~Graphics() = 0;

protected:
  void dispatchEvent();
};

std::shared_ptr<GraphicsEventListener> makeDefaultEventListner();
} // namespace VGG
