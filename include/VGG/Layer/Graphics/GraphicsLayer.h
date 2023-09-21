#pragma once
#include "GraphicsContext.h"
#include <memory>
#include <optional>

namespace VGG::layer
{

struct LayerConfig
{
  layer::GraphicsContext* context{ nullptr };
  float drawableSize[2];
  float dpi{ 1.0 };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

enum class ELayerError
{
  TEXTURE_SIZE_OUT_OF_RANGE,
  MAKE_CURRENT_CONTEXT_ERROR,
  RENDER_ENGINE_ERROR,
  UNKNOWN_ERROR,
};

class GraphicsLayer : public std::enable_shared_from_this<GraphicsLayer>
{
  layer::GraphicsContext* m_ctx{ nullptr };

public:
  std::optional<ELayerError> init(layer::GraphicsContext* ctx)
  {
    m_ctx = ctx;
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

  // After init, you must call resize with proper value so that render() could be called
  virtual void resize(int w, int h) = 0;
  virtual ~GraphicsLayer() = default;

protected:
  virtual std::optional<ELayerError> onInit() = 0;
};

} // namespace VGG::layer
