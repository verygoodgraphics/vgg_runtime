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

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include "Application/ElementAddProperty.hpp"
#include "Application/ElementDeleteProperty.hpp"
#include "Application/ElementGetProperty.hpp"
#include "Application/ElementUpdateProperty.hpp"
#include "Application/Event/EventListener.hpp"
#include "Application/UIAnimation.hpp"
#include "Application/UIOptions.hpp"
#include "Domain/Layout/LayoutContext.hpp"
#include "Domain/Layout/Rect.hpp"
#include "Event/Event.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Memory/Ref.hpp"
#include "UIEvent.hpp"
#include "glm/ext/vector_float2.hpp"
namespace VGG
{
class LayoutNode;
class StateTree;
namespace app
{
class AppRender;
}
namespace internal
{
class UIViewImpl;
}
namespace layer
{
class SceneNode;
}
struct ViewModel;
} // namespace VGG

namespace VGG
{

struct FontInfo
{
  std::string familyName;
  std::string subfamilyName;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FontInfo, familyName, subfamilyName);
class UIView : public app::EventListener
{
public:
  using EventListener = std::function<void(UIEventPtr, std::weak_ptr<LayoutNode>)>;
  using ResourcesType = std::map<std::string, std::vector<char>>;
  using HasEventListener = std::function<bool(const std::string&, EUIEventType)>;

  using ScalarType = int;

private:
  friend class Editor;
  using TargetNode = std::pair<std::shared_ptr<LayoutNode>, std::string>;

  std::unique_ptr<internal::UIViewImpl> m_impl;

  struct EventContext
  {
    TargetNode                       mouseOverTargetNode;
    std::shared_ptr<VGG::LayoutNode> mouseOverNode;
    TargetNode                       mouseEnterTargetNode;
    TargetNode                       mouseOutTargetNode;
    std::shared_ptr<VGG::LayoutNode> mouseOutNode;
    std::vector<TargetNode>          mouseLeaveTargetNodes; // enter nodes stack
  };

  EventListener    m_eventListener;
  HasEventListener m_hasEventListener;
  HasEventListener m_updateCursorEventListener;

  UIView*                              m_superview{ nullptr };
  std::vector<std::shared_ptr<UIView>> m_subviews;

  std::weak_ptr<LayoutNode> m_document;

  std::stack<std::string> m_history; // page id stack; excludes presented pages

  std::unordered_map<std::string, std::string> m_presentedPages;  // key: from; value: to
  std::unordered_map<std::string, std::string> m_presentingPages; // key: to; value: from

  // instance states
  std::vector<std::shared_ptr<VGG::StateTree>> m_stateTrees;

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
  bool m_paintOnceMoreAfterAnimation{ false };
  bool m_skipUntilNextLoop{ false };
  bool m_isZoomerEnabled{ true };
  bool m_drawGrayBackground{ false };

  std::optional<UEvent> m_lastMouseMove; // Used to trigger mouseenter event when content is updated

public:
  UIView();
  ~UIView();

  void frame();

  void show(
    std::shared_ptr<ViewModel>&                viewModel,
    bool                                       force = false,
    std::unordered_map<std::string, FontInfo>* requiredFonts = nullptr);
  void setOffsetAndScale(float xOffset, float yOffset, float scale);
  void resetOffsetAndScale();

  void registerEventListener(HasEventListener hasEventListener)
  {
    m_hasEventListener = hasEventListener;
  }

  void setEventListener(EventListener listener)
  {
    m_eventListener = listener;
  }

  void setUpdateCursorEventListener(HasEventListener listener)
  {
    m_updateCursorEventListener = listener;
  }

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

  bool isDirty();
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

  int currentPageIndex();

  void initHistory();
  bool setCurrentFrameIndex(
    const std::size_t                   index,
    const bool                          updateHistory,
    const app::UIAnimationOption&       option = {},
    const VGG::app::AnimationCompletion completion = {});
  bool pushFrame(
    const std::size_t                   index,
    const bool                          updateHistory,
    const app::UIAnimationOption&       option,
    const VGG::app::AnimationCompletion completion);
  bool presentFrame(
    const std::size_t        index,
    const app::FrameOptions& opts,
    app::AnimationCompletion completion);
  bool dismissFrame(const app::FrameOptions& opts, app::AnimationCompletion completion);
  bool popFrame(
    const app::PopOptions&        popOpts,
    const app::UIAnimationOption& animationOpts,
    app::AnimationCompletion      completion);

  void                       saveState(const std::shared_ptr<StateTree>& stateTree);
  std::shared_ptr<StateTree> savedState(const std::string& instanceDescendantId);
  void                       updateState(const LayoutNode* instanceNode);

  void enableZoomer(bool enabled);

  void setDrawBackground(bool drawBackground);
  void setBackgroundColor(uint32_t color);

  void triggerMouseEnter();

  void setLayer(app::AppRender* layer);

  layer::Ref<layer::SceneNode> getSceneNode();

public:
  bool setInstanceState(
    const LayoutNode*             oldNode,
    const LayoutNode*             newNode,
    const app::UIAnimationOption& options,
    app::AnimationCompletion      completion);
  bool presentInstanceState(
    const LayoutNode*             oldNode,
    const LayoutNode*             newNode,
    const app::UIAnimationOption& options,
    app::AnimationCompletion      completion);

public:
  std::unique_ptr<LayoutContext> layoutContext();

public:
  int updateElement(
    const std::vector<app::ElementUpdateProperty>& items,
    const app::UIAnimationOption&                  option);
  std::optional<app::ElementProperty> getElementProperty(const app::ElementGetProperty& query);
  bool                                addElementProperty(const app::ElementAddProperty& command);
  bool deleteElementProperty(const app::ElementDeleteProperty& command);

protected:
  float     scale() const;
  glm::vec2 offset() const;
  void      setOffset(float dx, float dy);
  void      translate(float dx, float dy);

private:
  struct EventHandleResult
  {
    bool handled = false;
    bool hasTarget = false;
  };

  std::tuple<bool, bool, bool, bool> getKeyModifier(int keyMod);

  void                        layoutSubviews();
  Layout::Point               converPointFromWindowAndScale(int x, int y);
  std::shared_ptr<LayoutNode> currentPage();
  std::shared_ptr<LayoutNode> pageById(const std::string& id);
  const std::string           pageIdByIndex(std::size_t index);

  void onMouseMove(UEvent evt, bool forHover = false);
  bool handleMouseEvent(
    int          jsButtonIndex,
    int          x,
    int          y,
    int          motionX,
    int          motionY,
    EUIEventType type,
    bool         forHover = false);
  EventHandleResult dispatchMouseEventOnPage(
    std::shared_ptr<LayoutNode> page,
    int                         jsButtonIndex,
    int                         x,
    int                         y,
    int                         motionX,
    int                         motionY,
    EUIEventType                type,
    bool                        updateCursor = false);
  void handleMouseOut(
    EventContext&                    eventContext,
    TargetNode                       targetNode,
    std::shared_ptr<VGG::LayoutNode> hitNodeInTarget,
    int                              jsButtonIndex,
    int                              x,
    int                              y,
    int                              motionX,
    int                              motionY);
  bool handleMouseLeave(
    EventContext& eventContext,
    TargetNode    target,
    Layout::Point pointToDocument,
    int           jsButtonIndex,
    Layout::Point pointToPage,
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

  void restoreState(const std::string& instanceId);

  void show(
    std::shared_ptr<ViewModel>&  viewModel,
    std::vector<layer::FramePtr> frames,
    bool                         force = false);
};

} // namespace VGG
