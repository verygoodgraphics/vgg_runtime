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

class Renderable : public std::enable_shared_from_this<Renderable>
{
public:
  virtual void onRender(SkCanvas* canvas) = 0;
};

class VLayer : public Graphics
{
  VGG_DECL_IMPL(VLayer);
  float m_scale{ 1.0 };
  float m_dpi{ 1.0 };

public:
  VLayer();
  ~VLayer();
  virtual std::optional<ELayerError> init(const LayerConfig& cfg) override;
  virtual void beginFrame() override;
  virtual void render() override;
  virtual void endFrame() override;
  virtual void shutdown() override;
  void addRenderItem(std::shared_ptr<Renderable> item);

  void setScale(float scale)
  {
    m_scale = scale;
  }

  float scale() const
  {
    return m_scale;
  }
  void setDPI(float dpi)
  {
    m_dpi = dpi;
  }
  float dpi() const
  {
    return m_dpi;
  }
  std::optional<std::vector<char>> makeImageSnapshot(const ImageOptions& opts);

protected:
  void onResizeEvent(int w, int h);
  bool onEvent(UEvent e) override;
};

} // namespace VGG
