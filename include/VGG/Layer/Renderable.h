#pragma once
#include "VGG/Layer/Config.h"

#include <memory>
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
  bool m_visible{ true };

protected:
  virtual void onRender(SkCanvas* canvas) = 0;

public:
  void render(SkCanvas* canvas);
  void setViewport(const Viewport& vp)
  {
    m_viewport = vp;
  }

  void setVisible(bool enable)
  {
    m_visible = enable;
  }

  bool visible() const
  {
    return m_visible;
  }
};
} // namespace VGG::layer
