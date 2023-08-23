#pragma once

#include <Utility/interface/Log.h>
#include <memory>
#include <vector>
namespace VGG::layer
{

class GraphicsLayer;
struct ContextConfig
{
  int windowSize[2] = { 0, 0 };
  float scaleFactor{ 1.0f };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

struct ContextProperty
{
  float resolutionScale{ 1.0 };
};

class GraphicsContext : public std::enable_shared_from_this<GraphicsContext>
{
  ContextConfig m_config;
  ContextProperty m_property;
  std::vector<std::shared_ptr<GraphicsLayer>> m_managedLayer;
  friend class GraphicsLayer;

protected:
  // this is the scale factor between window size and the surface size
  float m_resolutionScale{ 1.0 };

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
    auto ok = onInit();
    onInitProperties(m_property);
    return ok;
  }
  const ContextConfig& config() const
  {
    return m_config;
  }

  const ContextProperty& property() const
  {
    return m_property;
  }
  virtual bool swap() = 0;
  virtual bool makeCurrent() = 0;
  virtual void shutdown() = 0;
  bool resize(int w, int h);

protected:
  virtual bool onResize(int w, int h) = 0;
  virtual bool onInit() = 0;
  virtual void onInitProperties(ContextProperty& property) = 0;
};
} // namespace VGG::layer
