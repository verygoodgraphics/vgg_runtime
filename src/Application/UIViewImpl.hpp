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

#include <stdint.h>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include "Application/Animate.hpp"
#include "Application/ElementAddProperty.hpp"
#include "Application/ElementDeleteProperty.hpp"
#include "Application/ElementGetProperty.hpp"
#include "Application/ElementUpdateProperty.hpp"
#include "Application/UIAnimation.hpp"
#include "Application/ZoomerNodeController.hpp"
#include "Domain/Layout/LayoutContext.hpp"
#include "Layer/Core/FrameNode.hpp"
#include "Layer/Core/SceneNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Memory/Ref.hpp"
#include "Pager.hpp"
#include "glm/ext/vector_float2.hpp"

union UEvent;

namespace VGG
{
class LayoutNode;
class UIView;
struct ViewModel;
namespace app
{
class AppRender;
}

namespace internal
{

class UIViewImpl
{
public:
  struct UpdateBuilder
  {
    std::shared_ptr<AttrBridge>    updater;
    std::shared_ptr<LayoutNode>    layoutNode;
    layer::PaintNode*              paintNode = nullptr;
    std::shared_ptr<NumberAnimate> animation;
  };
  struct CommandContext
  {
    std::shared_ptr<AttrBridge> updater;
    std::shared_ptr<LayoutNode> layoutNode;
    layer::PaintNode*           paintNode = nullptr;
  };

public:
  UIViewImpl(UIView* api);

  void setLayer(app::AppRender* layer);

  void show(std::shared_ptr<ViewModel>& viewModel, std::vector<layer::FramePtr> frames);

  int  page() const;
  bool setPageIndex(int index);
  bool setPageIndexAnimated(
    std::size_t                   index,
    const app::UIAnimationOption& option,
    app::AnimationCompletion      completion = app::AnimationCompletion());
  void nextPage();
  void previoustPage();

  bool onEvent(UEvent evt, void* userData);

  layer::Ref<layer::SceneNode> sceneNode();

public:
  void setOffsetAndScale(float xOffset, float yOffset, float scale);
  void resetOffsetAndScale();

public:
  float scale() const;
  void  setScale(float scale);

  glm::vec2 offset() const;
  void      setOffset(glm::vec2 offset);

  void translate(float dx, float dy);

public:
  bool setInstanceState(
    const LayoutNode*             oldNode,
    const LayoutNode*             newNode,
    const app::UIAnimationOption& options,
    app::AnimationCompletion      completion);

public:
  void setBackgroundColor(uint32_t color);

  std::unique_ptr<LayoutContext> layoutContext();

public:
  bool deleteFinishedAnimation();
  bool isAnimating();

public:
  int updateElement(
    const std::vector<app::ElementUpdateProperty>& items,
    const app::UIAnimationOption&                  option);
  std::optional<app::ElementProperty> getElementProperty(const app::ElementGetProperty& query);
  bool                                addElementProperty(const app::ElementAddProperty& command);
  bool deleteElementProperty(const app::ElementDeleteProperty& command);

  std::optional<UpdateBuilder> makeUpdateBuilder(
    const std::string&            id,
    const app::UIAnimationOption* anamation,
    Animate*                      parentAnimation);
  std::optional<CommandContext> makeCommandContext(const std::string& id);
  layer::PaintNode*             getPaintNode(const std::string& id);

private:
  bool isUnitTest() const;

  bool transition(
    const LayoutNode*             fromNode,
    const LayoutNode*             toNode,
    const app::UIAnimationOption& option,
    app::AnimationCompletion      completion,
    const bool                    makeNewPaintNode = false);

  void moveFramesToTopLeft();

private:
  UIView* m_api;

  app::AppRender*                          m_layer{ nullptr };
  layer::Ref<layer::SceneNode>             m_sceneNode;
  std::unique_ptr<Pager>                   m_pager;
  std::unique_ptr<app::ZoomNodeController> m_zoomController;
  layer::Ref<layer::ZoomerNode>            m_zoomer;
  std::shared_ptr<ViewModel>               m_viewModel;

  AnimateManage m_animationManager;

  int m_pageIndexCache{ 0 };
};

} // namespace internal
} // namespace VGG
