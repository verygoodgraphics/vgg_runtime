#pragma once

#include "AppRenderable.h"
#include "UIEvent.hpp"

#include <Domain/Layout/Node.hpp>

#include <memory>
#include <vector>

class SkCanvas;

namespace VGG
{

class Editor : public app::AppRenderable
{
  bool m_enabled{ false };
  std::weak_ptr<LayoutNode> m_selectedNode;

public:
  void enable(bool enabled)
  {
    m_enabled = enabled;
  }

  void handleUIEvent(UIEventPtr event, std::weak_ptr<LayoutNode> targetNode);

  void onRender(SkCanvas* canvas) override;
  bool onEvent(UEvent e, void* userData) override
  {
    return false;
  }

private:
  void drawBorder(SkCanvas* canvas, const LayoutNode* node);
  void drawCornerPoint(SkCanvas* canvas, const LayoutNode* node);
};

} // namespace VGG