#pragma once
#include <memory>
#include <Common/Config.h>
#include <optional>

class SkCanvas;

namespace VGG::layer
{

struct Viewport
{
  int position[2] = { 0, 0 };
  int extent[2] = { 0, 0 };
};

class VGG_EXPORTS Renderable : public std::enable_shared_from_this<Renderable>
{
  std::optional<Viewport> m_viewport;

protected:
  virtual void onRenderImpl(SkCanvas* canvas) = 0;

public:
  void onRender(SkCanvas* canvas);
  void setViewport(const Viewport& vp)
  {
    m_viewport = vp;
  }
};
} // namespace VGG::layer
