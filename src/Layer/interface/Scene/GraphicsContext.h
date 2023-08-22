#pragma once

#include <memory>
namespace VGG::layer
{

struct ContextConfig
{
  float drawableSize[2];
  float dpi{ 1.0 };
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

class GraphicsContext : public std::enable_shared_from_this<GraphicsContext>
{
  ContextConfig m_config;

public:
  bool initGraphicsContext(const ContextConfig& cfg)
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
  virtual bool resize(int w, int h) = 0;
  virtual void shutdown() = 0;
  float dpi();

protected:
  virtual bool onInit() = 0;
};
} // namespace VGG::layer
