#pragma once
#include <memory>
#include <optional>
#include "Core/Node.h"
#include "../../../Application/include/Event/EventListener.h"
#include "Zoomer.h"

namespace VGG
{

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
class Graphics : public std::enable_shared_from_this<Graphics>
{
public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) = 0;
  virtual void beginFrame() = 0;
  virtual void render() = 0;
  virtual void endFrame() = 0;
  virtual void shutdown() = 0;
};

} // namespace VGG
