#pragma once
#include <memory>
#include <optional>

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

class GraphicLayer : public std::enable_shared_from_this<GraphicLayer>
{
public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) = 0;
  virtual void dispatchEvent(LayerEvent event) = 0;
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
  virtual ~GraphicLayer() = 0;
};
