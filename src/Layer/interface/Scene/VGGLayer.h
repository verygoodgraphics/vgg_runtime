#include "Core/Node.h"
#include "Scene/GraphicsLayer.h"
#include <vector>
class SkSurface;

namespace VGG
{

class VGGLayer__pImpl;
class Scene;

enum class EImageEncode
{
  IE_PNG
};

struct ImageOptions
{
  EImageEncode encode;
};

class VGGLayer : public GraphicLayer
{
  VGG_DECL_IMPL(VGGLayer);

private:
  VGGLayer();

public:
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) override;
  virtual void dispatchEvent(LayerEvent event) override;
  virtual void beginFrame() override;
  virtual void render() override;
  virtual void endFrame() override;
  virtual void shutdown() override;

  SkSurface* skiaSurface();
  Scene* scene();
  std::optional<std::vector<char>> makeImageSnapshot(const ImageOptions& opts);
};

} // namespace VGG
