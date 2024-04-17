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

#include "AppScene.hpp"
#include "UIEvent.hpp"

#include "Domain/Layout/Rect.hpp"
#include "Layer/Scene.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <stack>
#include <tuple>
#include <vector>
#include <utility>

namespace VGG
{

class LayoutNode;
class StateTree;
struct ViewModel;

class UIView : public app::AppScene
{
public:
  using EventListener = std::function<void(UIEventPtr, std::weak_ptr<LayoutNode>)>;
  using ResourcesType = std::map<std::string, std::vector<char>>;
  using HasEventListener = std::function<bool(const std::string&, EUIEventType)>;

  using ScalarType = int;

  struct Offset
  {
    double x;
    double y;
  };

private:
  friend class Editor;
  using TargetNode = std::pair<std::shared_ptr<LayoutNode>, std::string>;

  struct EventContext
  {
    TargetNode                       mouseOverTargetNode;
    std::shared_ptr<VGG::LayoutNode> mouseOverNode;
    TargetNode                       mouseEnterTargetNode;
    TargetNode                       mouseOutTargetNode;
    std::shared_ptr<VGG::LayoutNode> mouseOutNode;
    TargetNode                       mouseLeaveTargetNode;
  };

  EventListener    m_eventListener;
  HasEventListener m_hasEventListener;

  UIView*                              m_superview{ nullptr };
  std::vector<std::shared_ptr<UIView>> m_subviews;

  std::weak_ptr<LayoutNode> m_document;
  int                       m_page{ 0 };

  std::stack<std::string> m_history; // page id stack; excludes presented pages

  std::unordered_map<std::string, std::string> m_presentedPages;  // key: from; value: to
  std::unordered_map<std::string, std::string> m_presentingPages; // key: to; value: from

  // instance state
  std::shared_ptr<VGG::StateTree> m_stateTree;

  Layout::Rect m_frame;
  Layout::Rect m_bounds;

  TargetNode m_possibleClickTargetNode;

  std::unordered_map<std::string, EventContext> m_presentedTreeContext; // key: node id

  // editor
  bool       m_isEditor = false;
  // sidebar
  ScalarType m_top{ 0 };
  ScalarType m_right{ 0 };
  ScalarType m_bottom{ 0 };
  ScalarType m_left{ 0 };

  bool m_isDirty{ false };
  bool m_skipUntilNextLoop{ false };
  bool m_isZoomerEnabled{ true };
  bool m_drawGrayBackground{ false };

public:
  UIView();

  void show(const ViewModel& viewModel, bool force = false);
  void show(const ViewModel& viewModel, std::vector<layer::FramePtr> frames, bool force = false);
  void fitContent(float xOffset, float yOffset, float scale);
  void fitCurrentPage();

  void registerEventListener(HasEventListener hasEventListener)
  {
    m_hasEventListener = hasEventListener;
  }

  void setEventListener(EventListener listener)
  {
    m_eventListener = listener;
  }

  void onRender(SkCanvas* canvas) override;
  bool onEvent(UEvent e, void* userData) override;

  void addSubview(std::shared_ptr<UIView> view)
  {
    m_subviews.push_back(view);
    view->m_superview = this;
    layoutSubviews();
  }

  Layout::Size size()
  {
    return m_frame.size;
  }
  void setSize(ScalarType w, ScalarType h)
  {
    m_frame.size.width = w;
    m_frame.size.height = h;

    setDirty(true);

    layoutSubviews();
  }

  void becomeEditorWithSidebar(
    ScalarType top,
    ScalarType right,
    ScalarType bottom,
    ScalarType left);

  bool isDirty()
  {
    return m_isDirty;
  }
  void setDirty(const bool dirty)
  {
    if (dirty == m_isDirty)
    {
      return;
    }

    m_isDirty = dirty;
  }

  void updateOncePerLoop()
  {
    m_skipUntilNextLoop = false;
  }

  void nextArtboard();
  void preArtboard();
  int  currentPageIndex()
  {
    return m_page;
  }
  bool setCurrentPage(int index);
  bool back();
  bool presentPage(int index);
  bool dismissPage();

  void                       saveState(std::shared_ptr<StateTree> stateTree);
  std::shared_ptr<StateTree> savedState();
  void                       restoreState();

  void enableZoomer(bool enabled);

  void setDrawBackground(bool drawBackground)
  {
    m_drawGrayBackground = drawBackground;
  }

protected:
  Offset getOffset();
  void   setOffset(Offset offset);

private:
  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);

  void                        layoutSubviews();
  Layout::Point               converPointFromWindowAndScale(Layout::Point point);
  std::shared_ptr<LayoutNode> currentPage();
  std::shared_ptr<LayoutNode> pageById(const std::string& id);

  bool setCurrentPageIndex(int index);

  bool handleMouseEvent(
    int          jsButtonIndex,
    int          x,
    int          y,
    int          motionX,
    int          motionY,
    EUIEventType type);
  bool dispatchMouseEventOnPage(
    std::shared_ptr<LayoutNode> page,
    int                         jsButtonIndex,
    int                         x,
    int                         y,
    int                         motionX,
    int                         motionY,
    EUIEventType                type);
  void handleMouseOut(
    EventContext&                    eventContext,
    TargetNode                       targetNode,
    std::shared_ptr<VGG::LayoutNode> hitNodeInTarget,
    int                              jsButtonIndex,
    int                              x,
    int                              y,
    int                              motionX,
    int                              motionY);
  void handleMouseLeave(
    EventContext& eventContext,
    TargetNode    target,
    int           jsButtonIndex,
    int           x,
    int           y,
    int           motionX,
    int           motionY);
  void fireMouseEvent(
    TargetNode        target,
    VGG::EUIEventType type,
    int               jsButtonIndex,
    int               x,
    int               y,
    int               motionX,
    int               motionY);

  bool handleTouchEvent(int x, int y, int motionX, int motionY, EUIEventType type);
};

} // namespace VGG
