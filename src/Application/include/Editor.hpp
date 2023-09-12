#pragma once

#include "AppRenderable.h"
#include "UIEvent.hpp"

#include <Domain/Layout/Node.hpp>
#include <Log.h>

#include <memory>
#include <vector>

class SkCanvas;

namespace VGG
{

class UIView;

class Editor : public app::AppRenderable
{
  enum class EFramePosition
  {
    NONE,

    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,

    TOP,
    BOTTOM,
    LEFT,
    RIGHT
  };

  bool m_enabled{ false };
  std::weak_ptr<LayoutNode> m_hoverNode;
  std::weak_ptr<LayoutNode> m_selectedNode;
  std::weak_ptr<UIView> m_contentView;

  bool m_isMouseDown{ false };
  EFramePosition m_mouseDownPostion;

public:
  Editor(std::weak_ptr<UIView> contentView)
    : m_contentView{ contentView }
  {
    ASSERT(!contentView.expired());
  }

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

  void checkMouseDownPostion(MouseEvent* mouseDown);
  void resizeNode(MouseEvent* mouseMove);

  Layout::Rect getSelectNodeRect(EFramePosition position);
};

} // namespace VGG