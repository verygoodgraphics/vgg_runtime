#pragma once

#include "Utility/Log.h"
#include <memory>
#include <vector>
#include <optional>

namespace VGG::layer
{

class GraphicsLayer;

enum class EGraphicsAPIBackend
{
  API_OPENGL,
  API_VULKAN
};

struct ContextConfig
{
  // int windowSize[2] = { 0, 0 };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

struct ContextProperty
{
  float dpiScaling{ 1.0 };
  std::optional<EGraphicsAPIBackend> api;
};

class GraphicsContext : public std::enable_shared_from_this<GraphicsContext>
{
private:
  ContextConfig m_config;
  ContextProperty m_property;
  friend class GraphicsLayer;

protected:
  // this is the scale factor between window size and the surface size
  float m_resolutionScale{ 1.0 };

protected:
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
  virtual void* contextInfo() = 0;
  virtual ~GraphicsContext() = default;

protected:
  virtual bool onInit() = 0;
  virtual void onInitProperties(ContextProperty& property) = 0;
  virtual void onInitContext()
  {
  }
};
} // namespace VGG::layer
