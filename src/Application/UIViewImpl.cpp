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

#include "UIViewImpl.hpp"
#include <chrono>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include "Animate.hpp"
#include "AppRender.hpp"
#include "Application/AppLayoutContext.hpp"
#include "Application/Pager.hpp"
#include "Application/ElementGetProperty.hpp"
#include "Application/ElementUpdateProperty.hpp"
#include "Application/ViewModel.hpp"
#include "AttrBridge.hpp"
#include "Domain/Layout/LayoutNode.hpp"
#include "Domain/Layout/Rect.hpp"
#include "Domain/Model/Element.hpp"
#include "Event/Event.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/MemoryResourceProvider.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Core/ResourceManager.hpp"
#include "Layer/Core/ResourceProvider.hpp"
#include "Layer/Core/VColor.hpp"
#include "UIAnimation.hpp"
#include "UIView.hpp"
#include "Utility/Log.hpp"
#include "ZoomerNodeController.hpp"
#include <glm/detail/qualifier.hpp>

#undef DEBUG
#define DEBUG(msg, ...)

namespace VGG::internal
{

namespace
{
constexpr auto K_ANIMATION_INTERVAL = 16;

struct UpdateBuilderVisitor
{
  UIViewImpl*                   viewImpl = nullptr;
  const app::UIAnimationOption* animationOption = nullptr;
  Animate*                      parentAnimation = nullptr;

  std::optional<UIViewImpl::UpdateBuilder> operator()(const std::monostate&) const
  {
    return std::nullopt;
  }

  std::optional<UIViewImpl::UpdateBuilder> operator()(const app::ElementUpdate& u) const
  {
    return viewImpl->makeUpdateBuilder(u.id, animationOption, parentAnimation);
  }
};

struct UpdateVisitor
{
  const UIViewImpl::UpdateBuilder& b;

  bool operator()(const std::monostate&) const
  {
    return false;
  }

  bool operator()(const app::ElementUpdateFillBlendMode& u) const
  {
    return b.updater->updateFillBlendMode(b.layoutNode, b.paintNode, u.index, u.mode, false);
  }
  bool operator()(const app::ElementUpdateFillColor& u) const
  {
    return b.updater->updateFillColor(
      b.layoutNode,
      b.paintNode,
      u.index,
      Model::Color{ .alpha = u.a, .red = u.r, .green = u.g, .blue = u.b },
      false,
      b.animation);
  }
  bool operator()(const app::ElementUpdateFillEnabled& u) const
  {
    return b.updater
      ->updateFillEnabled(b.layoutNode, b.paintNode, u.index, u.enabled, false, b.animation);
  }
  bool operator()(const app::ElementUpdateFillOpacity& u) const
  {
    return b.updater
      ->updateFillOpacity(b.layoutNode, b.paintNode, u.index, u.opacity, false, b.animation);
  }
  bool operator()(const app::ElementUpdatePatternImageFillRotation& u) const
  {
    return b.updater->updatePatternImageFillRotation(
      b.layoutNode,
      b.paintNode,
      u.index,
      u.degree,
      false,
      u.effectOnFill,
      b.animation);
  }
  bool operator()(const app::ElementUpdateMatrix& u) const
  {
    return b.updater->updateMatrix(
      b.layoutNode,
      b.paintNode,
      { u.a, u.b, u.c, u.d, u.tx, u.ty },
      false,
      b.animation);
  }
  bool operator()(const app::ElementUpdateOpacity& u) const
  {
    return b.updater->updateOpacity(b.layoutNode, b.paintNode, u.opacity, false, b.animation);
  }
  bool operator()(const app::ElementUpdateVisible& u) const
  {
    return b.updater->updateVisible(b.layoutNode, b.paintNode, u.visible, false);
  }
  bool operator()(const app::ElementUpdateSize& u) const
  {
    return b.updater->updateSize(b.layoutNode, b.paintNode, u.width, u.height, false, b.animation);
  }

  bool operator()(const app::ElementUpdatePatternImageFileName& u) const
  {
    return b.updater->updatePatternImageFileName(
      b.layoutNode,
      b.paintNode,
      u.index,
      u.newName,
      false,
      u.effectOnFill);
  }

  bool operator()(const app::ElementUpdatePatternImageFilters& u) const
  {
    return b.updater->updatePatternImageFilters(
      b.layoutNode,
      b.paintNode,
      u.index,
      u.imageFilters,
      false,
      u.effectOnFill);
  }
};

struct GetPaintNodeVisitor
{
  UIViewImpl* viewImpl = nullptr;

  layer::PaintNode* operator()(const std::monostate&) const
  {
    return nullptr;
  }

  layer::PaintNode* operator()(const app::BaseElementGet& p) const
  {
    return viewImpl->getPaintNode(p.id);
  }
};

struct GetElementPropertyVisitor
{
  layer::PaintNode* n;

  std::optional<app::ElementProperty> operator()(const std::monostate&) const
  {
    return std::nullopt;
  }

  std::optional<app::ElementProperty> operator()(const app::ElementGetFillBlendMode& p) const
  {
    if (auto r = AttrBridge::getFillBlendMode(n, p.index))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetFillColor& p) const
  {
    if (auto r = AttrBridge::getFillColor(n, p.index))
      return app::ElementProperty(
        app::ElementColor{ .a = (*r).a, .r = (*r).r, .g = (*r).g, .b = (*r).b });
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetFillEnabled& p) const
  {
    if (auto r = AttrBridge::getFillEnabled(n, p.index))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetFillOpacity& p) const
  {
    if (auto r = AttrBridge::getFillOpacity(n, p.index))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetFillPatternType& p) const
  {
    if (auto r = AttrBridge::getFillPatternType(n, p.index))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetFillSize& p) const
  {
    if (auto r = AttrBridge::getFillSize(n))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetFillType& p) const
  {
    if (auto r = AttrBridge::getFillType(n, p.index))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetMatrix& p) const
  {
    if (auto r = AttrBridge::getMatrix(n))
      return app::ElementProperty(app::ElementMatrix{ .a = (*r)[0],
                                                      .b = (*r)[1],
                                                      .c = (*r)[2],
                                                      .d = (*r)[3],
                                                      .tx = (*r)[4],
                                                      .ty = (*r)[5] });
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetOpacity& p) const
  {
    if (auto r = AttrBridge::getOpacity(n))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(
    const app::ElementGetPatternImageFillRotation& p) const
  {
    if (auto r = AttrBridge::getPatternRotation(n, p.index, p.effectOnFill))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetVisible& p) const
  {
    if (auto r = AttrBridge::getVisible(n))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetHeight& p) const
  {
    if (auto r = AttrBridge::getHeight(n))
      return app::ElementProperty(*r);
    return std::nullopt;
  }
  std::optional<app::ElementProperty> operator()(const app::ElementGetWidth& p) const
  {
    if (auto r = AttrBridge::getWidth(n))
      return app::ElementProperty(*r);
    return std::nullopt;
  }

  std::optional<app::ElementProperty> operator()(const app::ElementGetPatternImageFileName& p) const
  {
    if (auto r = AttrBridge::getPatternImageFileName(n, p.index, p.effectOnFill))
      return app::ElementProperty(*r);
    return std::nullopt;
  }

  std::optional<app::ElementProperty> operator()(const app::ElementGetPatternImageFilters& p) const
  {
    if (auto r = AttrBridge::getPatternImageFilters(n, p.index, p.effectOnFill))
      return app::ElementProperty(app::ImageFilter{ .exposure = (*r).exposure,
                                                    .contrast = (*r).contrast,
                                                    .saturation = (*r).saturation,
                                                    .temperature = (*r).temperature,
                                                    .tint = (*r).tint,
                                                    .highlight = (*r).highlight,
                                                    .shadow = (*r).shadow,
                                                    .hue = (*r).hue });
    return std::nullopt;
  }
};

struct MakeCommandContextVisitor
{
  UIViewImpl* viewImpl = nullptr;

  std::optional<UIViewImpl::CommandContext> operator()(const std::monostate&) const
  {
    return std::nullopt;
  }

  std::optional<UIViewImpl::CommandContext> operator()(const app::BaseElementAdd& p) const
  {
    return viewImpl->makeCommandContext(p.id);
  }

  std::optional<UIViewImpl::CommandContext> operator()(const app::BaseElementDelete& p) const
  {
    return viewImpl->makeCommandContext(p.id);
  }
};

struct ProcessCommandVisitor
{
  const UIViewImpl::CommandContext& c;

  bool operator()(const std::monostate&)
  {
    return false;
  }

  bool operator()(const app::ElementAddFill& p)
  {
    return c.updater->addFill(c.layoutNode, c.paintNode, p.value, false, p.index);
  }

  bool operator()(const app::ElementDeleteFill& p)
  {
    return c.updater->delFill(c.layoutNode, c.paintNode, false, p.index);
  }
};

} // namespace

bool UIViewImpl::isUnitTest() const
{
  return m_layer == nullptr;
}

UIViewImpl::UIViewImpl(UIView* api)
  : m_api(api)
{
  m_zoomer = layer::ZoomerNode::Make();
  m_zoomController = std::make_unique<app::ZoomNodeController>(m_zoomer);
}

void UIViewImpl::setLayer(app::AppRender* layer)
{
  m_layer = layer;
}

void UIViewImpl::show(std::shared_ptr<ViewModel>& viewModel, std::vector<layer::FramePtr> frames)
{
  ASSERT(viewModel);
  m_viewModel = viewModel;

  DEBUG("UIViewImpl::show: make scene node");
  m_sceneNode = layer::SceneNode::Make(std::move(frames));
  moveFramesToTopLeft();

  if (!isUnitTest())
    m_layer->setRenderNode(m_zoomer, m_sceneNode);
  m_pager = std::make_unique<Pager>(m_sceneNode.get());
  setPageIndex(page());

  const auto&                                        repo = m_viewModel->resources();
  std::unordered_map<std::string, std::vector<char>> data(
    std::make_move_iterator(repo.begin()),
    std::make_move_iterator(repo.end()));

  layer::setGlobalResourceProvider(
    std::make_unique<layer::MemoryResourceProvider>(std::move(data)));
}

int UIViewImpl::page() const
{
  return m_pageIndexCache; // use cached page index, m_pager will be created after each show()
}
bool UIViewImpl::setPageIndex(int index)
{
  ASSERT(m_pager);

  if (m_pager->page() == index)
    return false;

  m_pager->setPage(index);
  m_pageIndexCache = m_pager->page();

  return true;
}

bool UIViewImpl::setPageIndexAnimated(
  std::size_t                   index,
  const app::UIAnimationOption& option,
  app::AnimationCompletion      completion)
{
  const auto oldIndex = page();
  if (oldIndex == (int)index)
    return false;

  const auto& root = m_viewModel->layoutTree();
  if (!root)
    return false;
  const auto& pages = root->children();
  if (index >= pages.size())
    return false;

  const auto& fromPage = pages[oldIndex];
  const auto& toPage = pages[index];

  DEBUG(
    "UIViewImpl::setPageIndexAnimated: from page: %s, to page: %s",
    fromPage->id().c_str(),
    toPage->id().c_str());
  transition(
    fromPage.get(),
    toPage.get(),
    option,
    [this, index, completion](bool)
    {
      setPageIndex(index);
      if (completion)
        completion(true); // todo false for unfinished
    });

  return true;
}

void UIViewImpl::nextPage()
{
  ASSERT(m_pager);
  m_pager->nextFrame();
}
void UIViewImpl::previoustPage()
{
  ASSERT(m_pager);
  m_pager->prevFrame();
}

bool UIViewImpl::onEvent(UEvent evt, void* userData)
{
  if (!m_zoomController)
    return false;

  return m_zoomController->onEvent(evt, userData);
}

layer::Ref<layer::SceneNode> UIViewImpl::sceneNode()
{
  return m_sceneNode;
}

void UIViewImpl::setOffsetAndScale(float xOffset, float yOffset, float scale)
{
  setScale(scale);

  // set offset last
  setOffset({ xOffset, yOffset });
}
void UIViewImpl::resetOffsetAndScale()
{
  m_zoomer->setScale(layer::ZoomerNode::SL_1_1);

  // set offset last
  setOffset({ 0, 0 });
}

float UIViewImpl::scale() const
{
  return m_zoomer->scale();
}
void UIViewImpl::setScale(float scale)
{
  m_zoomer->setScale(scale);
}

glm::vec2 UIViewImpl::offset() const
{
  return m_zoomer->getOffset();
}
void UIViewImpl::setOffset(glm::vec2 offset)
{
  m_zoomer->setOffset(offset);
}

void UIViewImpl::translate(float dx, float dy)
{
  m_zoomer->setTranslate(dx, dy);
}

void UIViewImpl::setBackgroundColor(uint32_t color)
{
  if (m_layer)
    m_layer->setBackgroundColor(color);
}

bool UIViewImpl::deleteFinishedAnimation()
{
  return m_animationManager.deleteFinishedAnimate();
}

bool UIViewImpl::isAnimating()
{
  return m_animationManager.hasRunningAnimation();
}

bool UIViewImpl::setInstanceState(
  const LayoutNode*             oldNode,
  const LayoutNode*             newNode,
  const app::UIAnimationOption& options,
  app::AnimationCompletion      completion)
{
  if (!oldNode || !newNode)
    return false;

  return transition(oldNode, newNode, options, completion, true);
}

bool UIViewImpl::transition(
  const LayoutNode*             inFromNode,
  const LayoutNode*             inToNode,
  const app::UIAnimationOption& option,
  app::AnimationCompletion      completion,
  const bool                    makeNewPaintNode)
{
  ASSERT(inFromNode && inToNode);
  auto fromNode = const_cast<LayoutNode*>(inFromNode)->shared_from_this();
  auto toNode = const_cast<LayoutNode*>(inToNode)->shared_from_this();

  DEBUG(
    "UIViewImpl::transition: from node: %s, to node: %s",
    fromNode->id().c_str(),
    toNode->id().c_str());

  auto action = std::make_shared<AttrBridge>(
    std::static_pointer_cast<UIView>(m_api->shared_from_this()),
    m_animationManager);

  const int duration = option.duration * 1000;
  auto      timing = std::make_shared<LinearInterpolator>();

  std::shared_ptr<ReplaceNodeAnimate> animation;

  switch (option.type)
  {
    case app::EAnimationType::NONE:
    case app::EAnimationType::DEFAULT:
      break;

    case app::EAnimationType::DISSOLVE:
      animation = std::make_shared<DissolveAnimate>(
        std::chrono::milliseconds(duration),
        std::chrono::milliseconds(K_ANIMATION_INTERVAL),
        timing,
        action);
      break;

    case app::EAnimationType::SMART:
      animation = std::make_shared<SmartAnimate>(
        std::chrono::milliseconds(duration),
        std::chrono::milliseconds(K_ANIMATION_INTERVAL),
        timing,
        action);
      break;
  }

  if (animation)
    animation->addCallBackWhenStop(
      [completion]()
      {
        if (completion)
          completion(true); // todo false for unfinished
      });

  const auto success = action->replaceNode(
    fromNode->shared_from_this(),
    toNode->shared_from_this(),
    action->getPaintNode(fromNode->shared_from_this()),
    makeNewPaintNode ? nullptr : action->getPaintNode(toNode->shared_from_this()),
    /*isOnlyUpdatePaint=*/true,
    animation,
    makeNewPaintNode);

  if (success && !animation)
  {
    m_api->setDirty(true);

    if (completion)
      completion(true);
  }

  return success;
}

void UIViewImpl::moveFramesToTopLeft()
{
  const auto& frames = m_viewModel->layoutTree()->children();
  auto        layerBridge = std::make_shared<AttrBridge>(
    std::static_pointer_cast<UIView>(m_api->shared_from_this()),
    m_animationManager);
  for (auto& frame : frames)
  {
    auto paintNode = layerBridge->getPaintNode(frame);
    ASSERT(paintNode);

    if (auto maybeMatrix = layerBridge->getMatrix(paintNode))
    {
      auto origin = frame->bounds().origin;
      auto size = frame->bounds().size;
      auto newMatrix = TransformHelper::moveToWindowTopLeft(
        origin.x,
        -origin.y,
        size.width,
        size.height,
        *maybeMatrix);
      layerBridge->updateMatrix(frame, paintNode, newMatrix, false);
    }
  }
}

std::unique_ptr<LayoutContext> UIViewImpl::layoutContext()
{
  return std::make_unique<AppLayoutContext>(std::make_shared<AttrBridge>(
    std::static_pointer_cast<UIView>(m_api->shared_from_this()),
    m_animationManager));
}

std::optional<UIViewImpl::UpdateBuilder> UIViewImpl::makeUpdateBuilder(
  const std::string&            id,
  const app::UIAnimationOption* option,
  Animate*                      parentAnimation)
{
  const auto& root = m_viewModel->layoutTree();
  if (!root)
    return std::nullopt;

  auto layoutNode = root->findDescendantNodeById(id);
  if (!layoutNode)
    return std::nullopt;

  auto paintNode = m_sceneNode->nodeByID(layoutNode->elementNode()->idNumber());
  if (!paintNode)
    return std::nullopt;

  std::shared_ptr<NumberAnimate> animation;
  if (parentAnimation)
  {
    animation = std::make_shared<NumberAnimate>(parentAnimation);
    parentAnimation->addChildAnimate(animation);
  }
  else if (option && option->type != app::EAnimationType::NONE)
  {
    const int duration = option->duration * 1000;
    auto      timing = std::make_shared<LinearInterpolator>();
    animation = std::make_shared<NumberAnimate>(
      std::chrono::milliseconds(duration),
      std::chrono::milliseconds(K_ANIMATION_INTERVAL),
      timing);
  }

  auto updater = std::make_shared<AttrBridge>(
    std::static_pointer_cast<UIView>(m_api->shared_from_this()),
    m_animationManager);

  return UpdateBuilder{ updater, layoutNode->shared_from_this(), paintNode, animation };
}

int UIViewImpl::updateElement(
  const std::vector<app::ElementUpdateProperty>& items,
  const app::UIAnimationOption&                  option)
{
  int                            successCount = 0;
  std::shared_ptr<NumberAnimate> animation;
  for (auto& item : items)
  {
    if (const auto& b = std::visit(UpdateBuilderVisitor{ this, &option, animation.get() }, item);
        b.has_value())
    {
      if (!animation)
        animation = (*b).animation; // first animation is parent
      successCount += std::visit(UpdateVisitor{ *b }, item);
    }
  }

  return successCount;
}

std::optional<app::ElementProperty> UIViewImpl::getElementProperty(
  const app::ElementGetProperty& query)
{
  std::optional<app::ElementProperty> r;
  if (auto n = std::visit(GetPaintNodeVisitor{ this }, query))
    r = std::visit(GetElementPropertyVisitor{ n }, query);
  return r;
}

layer::PaintNode* UIViewImpl::getPaintNode(const std::string& id)
{
  const auto& root = m_viewModel->layoutTree();
  if (!root)
    return nullptr;

  auto layoutNode = root->findDescendantNodeById(id);
  if (!layoutNode)
    return nullptr;

  return m_sceneNode->nodeByID(layoutNode->elementNode()->idNumber());
}

bool UIViewImpl::addElementProperty(const app::ElementAddProperty& command)
{
  if (const auto& c = std::visit(MakeCommandContextVisitor{ this }, command); c.has_value())
    return std::visit(ProcessCommandVisitor{ *c }, command);
  return false;
}

bool UIViewImpl::deleteElementProperty(const app::ElementDeleteProperty& command)
{
  if (const auto& c = std::visit(MakeCommandContextVisitor{ this }, command); c.has_value())
    return std::visit(ProcessCommandVisitor{ *c }, command);
  return false;
}

std::optional<UIViewImpl::CommandContext> UIViewImpl::makeCommandContext(const std::string& id)
{
  const auto& root = m_viewModel->layoutTree();
  if (!root)
    return std::nullopt;

  auto layoutNode = root->findDescendantNodeById(id);
  if (!layoutNode)
    return std::nullopt;

  auto paintNode = m_sceneNode->nodeByID(layoutNode->elementNode()->idNumber());
  if (!paintNode)
    return std::nullopt;

  auto updater = std::make_shared<AttrBridge>(
    std::static_pointer_cast<UIView>(m_api->shared_from_this()),
    m_animationManager);

  return CommandContext{ updater, layoutNode->shared_from_this(), paintNode };
}

} // namespace VGG::internal
