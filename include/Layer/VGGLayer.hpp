#pragma once
#include "Layer/Graphics/GraphicsLayer.hpp"
#include "Layer/Scene.hpp"

#include <vector>
class SkCanvas;

namespace VGG::layer
{

enum class EImageEncode
{
  IE_PNG
};

struct ImageOptions
{
  EImageEncode encode;
  int position[2];
  int extend[2];
  int quality{ 100 };
};

class VLayer__pImpl;
class VLayer : public GraphicsLayer
{
  VGG_DECL_IMPL(VLayer);
  float m_scale{ 1.0 };
  bool m_drawPos{ false };
  int m_position[2] = { 0, 0 };

protected:
  virtual std::optional<ELayerError> onInit() override;

public:
  VLayer();
  ~VLayer();
  virtual void beginFrame() override;
  virtual void render() override;
  virtual void endFrame() override;
  virtual void shutdown() override;
  void resize(int w, int h) override;
  void addRenderItem(std::shared_ptr<Renderable> item);
  void addScene(std::shared_ptr<Scene> scene);
  void setScene(std::shared_ptr<Scene> scene);
  void setDrawPositionEnabled(bool enable)
  {
    m_drawPos = enable;
  }

  bool enableDrawPosition() const
  {
    return m_drawPos;
  }

  void drawPosition(int x, int y)
  {
    m_position[0] = x;
    m_position[1] = y;
  }

  void setScaleFactor(float scale)
  {
    m_scale = scale;
  }

  float scaleFactor() const
  {
    return m_scale;
  }

  std::optional<std::vector<char>> makeImageSnapshot(const ImageOptions& opts);
};

} // namespace VGG::layer
