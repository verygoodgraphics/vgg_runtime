#pragma once
#include "Scene/GraphicsContext.h"
#include <memory>
#include <optional>

namespace VGG
{

enum class EGraphicAPI
{
  GA_OPENGL
};

struct LayerConfig
{
  EGraphicAPI graphicsAPI{ EGraphicAPI::GA_OPENGL };
  layer::GraphicsContext* context{ nullptr };
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

class GraphicsLayer : public std::enable_shared_from_this<GraphicsLayer>
{
public:
  virtual std::optional<ELayerError> init(layer::GraphicsContext* ctx) = 0;
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
};

} // namespace VGG
