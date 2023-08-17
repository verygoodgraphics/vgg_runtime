#include "Core/Node.h"
#include "Scene/GraphicsLayer.h"
#include <vector>
class SkSurface;

namespace VGG
{

class VLayer__pImpl;
class Scene;

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
  SkSurface* skiaSurface();
  std::optional<std::vector<char>> makeImageSnapshot(const ImageOptions& opts);
};

} // namespace VGG
