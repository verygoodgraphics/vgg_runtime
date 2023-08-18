#pragma once
#include "Core/Node.h"
#include "Scene/GraphicsLayer.h"
#include <vector>

namespace VGG
{

class VLayer__pImpl;

enum class EImageEncode
{
  IE_PNG
};

struct ImageOptions
{
  EImageEncode encode;
};

class VLayer : public Graphics
{
  VGG_DECL_IMPL(VLayer);

private:
  VLayer();

public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) override;
  virtual void beginFrame() override;
  virtual void render() override;
  virtual void endFrame() override;
  virtual void shutdown() override;
  std::optional<std::vector<char>> makeImageSnapshot(const ImageOptions& opts);
  static std::shared_ptr<VLayer> makeVLayer(float dpi)
  {
    return nullptr;
  }

protected:
  void onResizeEvent(int w, int h);
  void onEvent(UEvent e) override;
};

} // namespace VGG
