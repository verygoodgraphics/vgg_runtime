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

#include "UIAnimation.hpp"

#include "Utility/Log.hpp"

using namespace VGG;
using namespace VGG::internal;
namespace
{
constexpr auto K_ANIMATION_INTERVAL = 16;
}

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

  m_sceneNode = layer::SceneNode::Make(std::move(frames));
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

  DEBUG("from page: %s, to page: %s", fromPage->id().c_str(), toPage->id().c_str());
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
  m_layer->setBackgroundColor(color);
}

void UIViewImpl::frame()
{
  m_animationManager.deleteFinishedAnimate();
}

bool UIViewImpl::isAnimating()
{
  return m_animationManager.hasRunningAnimation();
}

bool UIViewImpl::updateNodeFillColor(
  const std::string& id,
  const std::size_t  fillIndex,
  const double       r,
  const double       g,
  const double       b,
  const double       a)
{
  auto node = m_sceneNode->nodeByID(id);
  if (!node)
    return false;

  auto fills = node->attributeAccessor()->getFills();
  if (fillIndex >= fills.size())
    return false;

  Color color;
  color.a = a;
  color.r = r;
  color.g = g;
  color.b = b;
  fills.at(fillIndex).type = color;
  node->attributeAccessor()->setFills(fills);
  return true;
}

bool UIViewImpl::setInstanceState(
  const LayoutNode*             oldNode,
  const LayoutNode*             newNode,
  const app::UIAnimationOption& options,
  app::AnimationCompletion      completion)
{
  if (!oldNode || !newNode)
    return false;

  transition(oldNode, newNode, options, completion, true);
  return true;
}

void UIViewImpl::transition(
  const LayoutNode*             inFromNode,
  const LayoutNode*             inToNode,
  const app::UIAnimationOption& option,
  app::AnimationCompletion      completion,
  const bool                    makeNewPaintNode)
{
  ASSERT(inFromNode && inToNode);
  auto fromNode = const_cast<LayoutNode*>(inFromNode)->shared_from_this();
  auto toNode = const_cast<LayoutNode*>(inToNode)->shared_from_this();

  DEBUG("from node: %s, to node: %s", fromNode->id().c_str(), toNode->id().c_str());

  auto action = std::make_shared<AttrBridge>(
    std::static_pointer_cast<UIView>(m_api->shared_from_this()),
    m_animationManager);

  const int duration = option.duration * 1000;
  auto      timing = std::make_shared<LinearInterpolator>();

  std::shared_ptr<ReplaceNodeAnimate> animation;

  switch (option.type)
  {
    case app::EAnimationType::NONE:
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

  action->replaceNode(
    fromNode->shared_from_this(),
    toNode->shared_from_this(),
    action->getPaintNode(fromNode->shared_from_this()),
    makeNewPaintNode ? nullptr : action->getPaintNode(toNode->shared_from_this()),
    true,
    animation);

  if (!animation)
  {
    m_api->setDirty(true);

    if (completion)
      completion(true);
  }
}
