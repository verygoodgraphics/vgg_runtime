#pragma once

#include <Utility/interface/Log.h>
#include <memory>
#include <vector>
namespace VGG::layer
{

class GraphicsLayer;
struct ContextConfig
{
  int drawableSize[2] = { 0, 0 };
  float resolutionScale{ 1.0f };
  float scaleFactor{ 1.0f };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

class GraphicsContext : public std::enable_shared_from_this<GraphicsContext>
{
  ContextConfig m_config;
  std::vector<std::shared_ptr<GraphicsLayer>> m_managedLayer;
  friend class GraphicsLayer;

protected:
  void addManagedLayer(std::shared_ptr<GraphicsLayer> layer)
  {
    m_managedLayer.push_back(std::move(layer));
  }

  void removeFromManagedLayer(std::shared_ptr<GraphicsLayer> layer)
  {
    WARN("Not implemeted");
  }

public:
  bool init(const ContextConfig& cfg)
  {
    m_config = cfg;
    return onInit();
  }
  const ContextConfig& config() const
  {
    return m_config;
  }
  virtual bool swap() = 0;
  virtual bool makeCurrent() = 0;
  bool resize(int w, int h);
  virtual void shutdown() = 0;

protected:
  virtual bool onResize(int w, int h) = 0;
  virtual bool onInit() = 0;
};
} // namespace VGG::layer
