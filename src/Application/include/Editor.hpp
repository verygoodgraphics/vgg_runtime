#pragma once

#include "AppScene.h"
#include "UIEvent.hpp"

#include <Domain/Layout/Node.hpp>

#include <memory>
#include <vector>

class SkCanvas;

namespace VGG
{

class Editor : public app::AppScene
{
  bool m_enabled{ false };
  std::vector<std::weak_ptr<LayoutNode>> m_selectedNodes;

public:
  void enable(bool enabled)
  {
    m_enabled = enabled;
  }

  void handleUIEvent(UIEventPtr event);

  void onRender(SkCanvas* canvas) override;

private:
  void drawBorder(SkCanvas* canvas, const LayoutNode* node);
  void drawCornerPoint(SkCanvas* canvas, const LayoutNode* node);
};

} // namespace VGG