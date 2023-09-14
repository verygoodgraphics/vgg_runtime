#pragma once

#include "AppRenderable.h"
#include "UIEvent.hpp"

#include <Application/Mouse.hpp>
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
  enum class EResizePosition
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

  bool m_isEnabled{ false };
  std::weak_ptr<LayoutNode> m_hoverNode;
  std::weak_ptr<LayoutNode> m_selectedNode;
  std::weak_ptr<UIView> m_contentView;

  bool m_isMouseDown{ false };
  EResizePosition m_mouseDownPosition;

  std::shared_ptr<Mouse> m_mouse;

  bool m_isDirty{ false };

public:
  Editor(std::weak_ptr<UIView> contentView, std::shared_ptr<Mouse> mouse)
    : m_contentView{ contentView }
    , m_mouse{ mouse }
  {
    ASSERT(!contentView.expired());
    ASSERT(mouse);
  }

  void enable(bool enabled);

  void handleUIEvent(UIEventPtr event, std::weak_ptr<LayoutNode> targetNode);

  void onRender(SkCanvas* canvas) override;
  bool onEvent(UEvent e, void* userData) override
  {
    return false;
  }

  bool isDirty()
  {
    return m_isEnabled && m_isDirty;
  }
  void setDirty(bool dirty)
  {
    m_isDirty = dirty;
  }

private:
  void drawBorder(SkCanvas* canvas, const LayoutNode* node);

  EResizePosition checkMousePostion(int x, int y);
  void resizeNode(MouseEvent* mouseMove);

  Layout::Rect getSelectNodeRect(EResizePosition position);

  void updateCursor(EResizePosition mousePosition);
};

} // namespace VGG