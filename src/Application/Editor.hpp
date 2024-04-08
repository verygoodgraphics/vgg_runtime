/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "AppRenderable.hpp"
#include "UIEvent.hpp"

#include "Application/Mouse.hpp"
#include "Domain/Layout/Rect.hpp"
#include "Utility/Log.hpp"

#include <memory>
#include <vector>

class SkCanvas;

namespace VGG
{
class LayoutNode;
class UIView;

class Editor : public app::AppRenderable
{
public:
  class Listener
  {
  public:
    virtual ~Listener() = default;

    virtual void onSelectNode(std::weak_ptr<LayoutNode> node) = 0;
  };

private:
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

  bool                      m_isEnabled{ false };
  std::weak_ptr<LayoutNode> m_hoverNode;
  std::weak_ptr<LayoutNode> m_selectedNode;
  std::weak_ptr<UIView>     m_contentView;

  bool            m_isMouseDown{ false };
  EResizePosition m_mouseDownPosition;

  std::shared_ptr<Mouse> m_mouse;

  bool m_isDirty{ false };
  bool m_isModelDirty{ false };

  std::weak_ptr<Listener> m_listener;

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

  bool isDirty();
  bool isModelDirty();
  void resetModelDirty();

  void setListener(std::weak_ptr<Listener> listener)
  {
    m_listener = listener;
  }

private:
  void drawBorder(SkCanvas* canvas, const LayoutNode* node);

  EResizePosition checkMousePostion(int x, int y);
  void            resizeNode(MouseEvent* mouseMove);

  Layout::Rect getSelectNodeRect(EResizePosition position);

  void updateCursor(EResizePosition mousePosition);
  void didSelectNode(std::weak_ptr<LayoutNode> node);
  void setDirty(const bool dirty);
};

} // namespace VGG
