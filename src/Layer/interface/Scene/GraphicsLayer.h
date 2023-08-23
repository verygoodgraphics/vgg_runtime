#pragma once
#include "Scene/GraphicsContext.h"
#include <memory>
#include <optional>

namespace VGG::layer
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
  layer::GraphicsContext* m_ctx{ nullptr };

public:
  std::optional<ELayerError> init(layer::GraphicsContext* ctx)
  {
    m_ctx = ctx;
    m_ctx->addManagedLayer(shared_from_this());
    return onInit();
  }
  layer::GraphicsContext* context()
  {
    return m_ctx;
  }
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
  virtual void resize(int w, int h) = 0;
  virtual ~GraphicsLayer()
  {
    // remove from mananged layer from context
    // m_ctx->removeFromManagedLayer(shared_from_this());
  }

protected:
  virtual std::optional<ELayerError> onInit() = 0;
};

} // namespace VGG::layer
