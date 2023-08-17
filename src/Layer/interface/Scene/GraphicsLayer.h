#pragma once
#include <memory>
#include <optional>
#include "Core/Node.h"
#include "Scene/EventListener.h"

#include "../../../Application/include/Event/Event.h"

namespace VGG
{

enum class EGraphicAPI
{
  GA_OPENGL
};

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

class Graphics__pImpl;
class Graphics : public std::enable_shared_from_this<Graphics>
{
  VGG_DECL_IMPL(Graphics)
public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) = 0;
  void pushEvent(UEvent e);
  void addEventListener(std::shared_ptr<EventListener> listener);
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
  virtual ~Graphics() = 0;

protected:
  Graphics();
  virtual void dispatchEvent();
};

} // namespace VGG
