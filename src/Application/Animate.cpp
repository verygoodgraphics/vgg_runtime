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
#include "Application/Animate.hpp"
#include "Application/AttrBridge.hpp"
#include "Utility/VggTimer.hpp"
#include "Utility/Log.hpp"
#include "Domain/Layout/Node.hpp"
#include "Domain/Model/Element.hpp"
#include "Layer/Core/PaintNode.hpp"
#include <unordered_map>

using namespace VGG;

void clearPaintNodeChildren(layer::PaintNode* node)
{
  if (!node)
  {
    return;
  }

  while (node->begin() != node->end())
  {
    auto it = node->end();
    node->removeChild(--it);
  }
}

void setPaintNodeChildren(layer::PaintNode* node, const layer::PaintNode::ChildContainer& children)
{
  clearPaintNodeChildren(node);
  for (auto& child : children)
  {
    node->addChild(child);
  }
}

Interpolator::~Interpolator()
{
}

auto Interpolator::isFinished()
{
  return m_finished;
}

void Interpolator::setFinished()
{
  m_finished = true;
}

double LinearInterpolator::operator()(
  milliseconds passedTime,
  double       from,
  double       to,
  milliseconds duration)
{
  assert(duration.count());

  if (passedTime >= duration || !duration.count())
  {
    setFinished();
    return to;
  }

  return (to - from) * passedTime.count() / duration.count() + from;
}

Animate::Animate(
  milliseconds                  duration,
  milliseconds                  interval,
  std::shared_ptr<Interpolator> interpolator)
  : m_duration(duration)
  , m_interval(interval)
  , m_interpolator(interpolator)
{
}

Animate::~Animate()
{
  stop();
}

void Animate::stop()
{
  if (m_timer)
  {
    m_timer->invalidate();
    m_timer = nullptr;
  }

  if (!m_callbackWhenStop.empty())
  {
    for (auto& item : m_callbackWhenStop)
    {
      item();
    }
    m_callbackWhenStop.clear();
  }
}

bool Animate::isRunning()
{
  return static_cast<bool>(m_timer);
}

auto Animate::getDuration()
{
  return m_duration;
}

auto Animate::getInterval()
{
  return m_interval;
}

auto Animate::getInterpolator()
{
  return m_interpolator;
}

auto Animate::getStartTime()
{
  return m_startTime;
}

bool Animate::isFinished()
{
  return getInterpolator()->isFinished() || !isRunning();
}

void Animate::start()
{
  assert(!m_timer);
  m_startTime = std::chrono::steady_clock::now();
}

std::shared_ptr<Timer>& Animate::getTimerRef()
{
  return m_timer;
}

void Animate::addCallBackWhenStop(std::function<void()>&& fun)
{
  m_callbackWhenStop.emplace_back(std::move(fun));
}

void AnimateManage::addAnimate(std::shared_ptr<Animate> animate)
{
  m_animates.emplace_back(animate);
}

bool AnimateManage::hasRunningAnimation() const
{
  for (const auto& item : m_animates)
    if (item->isRunning())
      return true;

  return false;
}

void AnimateManage::deleteFinishedAnimate()
{
  auto it = std::remove_if(
    m_animates.begin(),
    m_animates.end(),
    [](auto animate)
    {
      auto result = animate->isFinished();
      if (result)
      {
        animate->stop();
      }
      return result;
    });

  if (it != m_animates.end())
  {
    DEBUG("animate stopped");
    m_animates.erase(it, m_animates.end());
  }
}

NumberAnimate::NumberAnimate(
  milliseconds                  duration,
  milliseconds                  interval,
  std::shared_ptr<Interpolator> interpolator)
  : Animate(duration, interval, interpolator)
{
}

void NumberAnimate::addTriggeredCallback(TTriggeredCallback&& fun)
{
  m_triggeredCallback.emplace_back(std::move(fun));
}

const std::vector<NumberAnimate::TTriggeredCallback>& NumberAnimate::getTriggeredCallback()
{
  return m_triggeredCallback;
}

void NumberAnimate::start()
{
  Animate::start();

  auto& timer = getTimerRef();
  timer = std::make_shared<Timer>(
    getInterval().count() / 1000.0,
    [this]()
    {
      auto interpolator = getInterpolator();
      assert(interpolator);

      auto passedTime =
        std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now() - getStartTime());
      const auto size = m_nowValue.size();
      assert(size && size == m_from.size() && size == m_to.size());

      for (size_t i = 0; i < size; ++i)
      {
        m_nowValue[i] = (*interpolator)(passedTime, m_from.at(i), m_to.at(i), getDuration());
      }

      m_action(m_nowValue);

      for (auto& item : this->getTriggeredCallback())
      {
        item(m_nowValue);
      }
    },
    true);
}

void NumberAnimate::setFromTo(const TParam& from, const TParam& to)
{
  assert(!from.empty() && from.size() == to.size());
  m_from = from;
  m_to = to;
  m_nowValue.resize(from.size());
}

void NumberAnimate::setAction(std::function<void(const TParam&)> action)
{
  m_action = action;
}

ReplaceNodeAnimate::ReplaceNodeAnimate(
  milliseconds                  duration,
  milliseconds                  interval,
  std::shared_ptr<Interpolator> interpolator,
  std::shared_ptr<AttrBridge>   attrBridge)
  : Animate(duration, interval, interpolator)
  , m_attrBridge(attrBridge)
  , m_isOnlyUpdatePaint(false)
{
}

bool ReplaceNodeAnimate::isRunning()
{
  return std::any_of(
    m_childAnimates.begin(),
    m_childAnimates.end(),
    [](std::shared_ptr<Animate> child) { return child->isRunning(); });
}

bool ReplaceNodeAnimate::isFinished()
{
  return std::all_of(
    m_childAnimates.begin(),
    m_childAnimates.end(),
    [](std::shared_ptr<Animate> child) { return child->isFinished(); });
}

bool ReplaceNodeAnimate::isContainerType(const Domain::Element* node)
{
  assert(node);

  auto type = node->type();
  if (
    type == VGG::Domain::Element::EType::FRAME || type == VGG::Domain::Element::EType::GROUP ||
    type == VGG::Domain::Element::EType::SYMBOL_INSTANCE ||
    type == VGG::Domain::Element::EType::SYMBOL_MASTER)
  {
    return true;
  }

  return false;
}

void ReplaceNodeAnimate::setFromTo(std::shared_ptr<LayoutNode> from, std::shared_ptr<LayoutNode> to)
{
  m_from = from;
  m_to = to;
}

auto ReplaceNodeAnimate::getFrom()
{
  return m_from;
}

auto ReplaceNodeAnimate::getTo()
{
  return m_to;
}

auto ReplaceNodeAnimate::getAttrBridge()
{
  return m_attrBridge;
}

auto ReplaceNodeAnimate::getIsOnlyUpdatePaint()
{
  return m_isOnlyUpdatePaint;
}

void ReplaceNodeAnimate::setIsOnlyUpdatePaint(bool isOnlyUpdatePaint)
{
  m_isOnlyUpdatePaint = isOnlyUpdatePaint;
}

std::shared_ptr<NumberAnimate> ReplaceNodeAnimate::createAndAddNumberAnimate()
{
  auto animate = std::make_shared<NumberAnimate>(getDuration(), getInterval(), getInterpolator());
  m_childAnimates.emplace_back(animate);
  return animate;
};

void ReplaceNodeAnimate::addStyleOpacityAnimate(
  std::shared_ptr<LayoutNode> node,
  layer::PaintNode*           paintNode,
  bool                        toVisible,
  bool                        recursive)
{
  if (!node || !node->elementNode() || !paintNode)
  {
    return;
  }

  auto attrBridge = getAttrBridge();
  auto isOnlyUpdatePaint = getIsOnlyUpdatePaint();

  // for fill
  do
  {
    auto size = *attrBridge->getFillSize(paintNode);
    for (size_t i = 0; i < size; ++i)
    {
      auto opacity = AttrBridge::getFillOpacity(paintNode, i);
      if (!opacity)
      {
        assert(false);
        continue;
      }

      auto animate = createAndAddNumberAnimate();

      if (toVisible)
      {
        attrBridge->updateFillOpacity(node, paintNode, i, 0, isOnlyUpdatePaint);
        attrBridge->updateFillOpacity(node, paintNode, i, *opacity, isOnlyUpdatePaint, animate);
      }
      else
      {
        attrBridge->updateFillOpacity(node, paintNode, i, 0, isOnlyUpdatePaint, animate);
        addCallBackWhenStop(
          [attrBridge, node, isOnlyUpdatePaint, paintNode, opacity, i]()
          { attrBridge->updateFillOpacity(node, paintNode, i, *opacity, isOnlyUpdatePaint); });
      }
    }
  } while (false);

  // TODO other attr in style

  if (recursive && ReplaceNodeAnimate::isContainerType(node->elementNode()))
  {
    auto& children = node->children();
    for (auto& child : children)
    {
      auto nodeChild = child->shared_from_this();
      addStyleOpacityAnimate(nodeChild, attrBridge->getPaintNode(nodeChild), toVisible, recursive);
    }
  }
}

void ReplaceNodeAnimate::addTwinMatrixAnimate(const TTwins& twins)
{
  auto attrBridge = getAttrBridge();
  bool isOnlyUpdatePaint = getIsOnlyUpdatePaint();

  for (auto& item : twins)
  {
    auto& itemFrom = item.first;
    auto& itemTo = item.second;

    if (!itemFrom || !itemTo)
    {
      assert(false);
      continue;
    }

    auto paintNodeFrom = attrBridge->getPaintNode(itemFrom);
    auto paintNodeTo = attrBridge->getPaintNode(itemTo);

    if (
      !paintNodeFrom || !paintNodeTo || !*AttrBridge::getVisible(paintNodeFrom) ||
      !*AttrBridge::getVisible(paintNodeTo))
    {
      continue;
    }

    auto transform = TransformHelper::transform(
      *AttrBridge::getWidth(paintNodeFrom),
      *AttrBridge::getHeight(paintNodeFrom),
      *AttrBridge::getWidth(paintNodeTo),
      *AttrBridge::getHeight(paintNodeTo),
      *AttrBridge::getMatrix(paintNodeTo));

    auto animate = createAndAddNumberAnimate();
    auto paintNodeFromOriginalMatrix = attrBridge->getMatrix(paintNodeFrom);
    bool itemFromIsContainer = ReplaceNodeAnimate::isContainerType(itemFrom->elementNode());

    attrBridge->updateMatrix(
      itemFrom,
      paintNodeFrom,
      transform,
      isOnlyUpdatePaint,
      animate,
      itemFromIsContainer);

    animate->addTriggeredCallback(std::bind(
      AttrBridge::setTwinMatrix,
      itemFrom,
      itemTo,
      paintNodeTo,
      std::placeholders::_1,
      isOnlyUpdatePaint));

    animate->addCallBackWhenStop(
      [paintNodeFromOriginalMatrix,
       itemFrom,
       paintNodeFrom,
       attrBridge,
       isOnlyUpdatePaint,
       itemFromIsContainer]()
      {
        assert(paintNodeFromOriginalMatrix);
        attrBridge->updateMatrix(
          itemFrom,
          paintNodeFrom,
          *paintNodeFromOriginalMatrix,
          isOnlyUpdatePaint,
          {},
          itemFromIsContainer);
      });
  }
}

DissolveAnimate::DissolveAnimate(
  milliseconds                  duration,
  milliseconds                  interval,
  std::shared_ptr<Interpolator> interpolator,
  std::shared_ptr<AttrBridge>   attrBridge)
  : ReplaceNodeAnimate(duration, interval, interpolator, attrBridge)
{
}

void DissolveAnimate::start()
{
  auto attrBridge = getAttrBridge();
  auto from = getFrom();
  auto to = getTo();
  auto isOnlyUpdatePaint = getIsOnlyUpdatePaint();
  assert(attrBridge && (from || to));

  if (to)
  {
    auto animate = createAndAddNumberAnimate();
    auto paintNodeTo = attrBridge->getPaintNode(to);
    auto opacity = AttrBridge::getOpacity(paintNodeTo);
    assert(opacity);
    attrBridge->updateOpacity(to, paintNodeTo, 0, isOnlyUpdatePaint);
    attrBridge->updateVisible(to, paintNodeTo, true, isOnlyUpdatePaint);
    attrBridge->updateOpacity(to, paintNodeTo, opacity ? *opacity : 1, isOnlyUpdatePaint, animate);
  }

  if (from)
  {
    auto animate = createAndAddNumberAnimate();
    auto paintNodeFrom = attrBridge->getPaintNode(from);
    auto opacity = AttrBridge::getOpacity(paintNodeFrom);
    assert(opacity);

    animate->addCallBackWhenStop(
      [attrBridge, from, isOnlyUpdatePaint, paintNodeFrom, opacity]()
      {
        attrBridge->updateVisible(from, paintNodeFrom, false, isOnlyUpdatePaint);
        attrBridge->updateOpacity(from, paintNodeFrom, opacity ? *opacity : 1, isOnlyUpdatePaint);
      });

    // attrBridge->updateVisible(from, true, isOnlyUpdatePaint);
    attrBridge->updateOpacity(from, paintNodeFrom, 0, isOnlyUpdatePaint, animate);
  }
}

SmartAnimate::SmartAnimate(
  milliseconds                  duration,
  milliseconds                  interval,
  std::shared_ptr<Interpolator> interpolator,
  std::shared_ptr<AttrBridge>   attrBridge)
  : ReplaceNodeAnimate(duration, interval, interpolator, attrBridge)
{
}

void SmartAnimate::start()
{
  auto from = getFrom();
  auto to = getTo();
  auto attrBridge = getAttrBridge();
  auto isOnlyUpdatePaint = getIsOnlyUpdatePaint();

  if (!from || !to || !attrBridge)
  {
    assert(false);
    return;
  }

  auto paintNodeFrom = attrBridge->getPaintNode(from);
  auto paintNodeTo = attrBridge->getPaintNode(to);
  if (!paintNodeFrom || !paintNodeTo)
  {
    assert(false);
    return;
  }

  addCallBackWhenStop(
    [attrBridge, from, isOnlyUpdatePaint, paintNodeFrom]()
    { attrBridge->updateVisible(from, paintNodeFrom, false, isOnlyUpdatePaint); });

  addTwinAnimate(from, to);
  addStyleOpacityAnimate(from, paintNodeFrom, false, true);
  addStyleOpacityAnimate(to, paintNodeTo, true, true);
  attrBridge->updateVisible(to, attrBridge->getPaintNode(to), true, isOnlyUpdatePaint);
}

void SmartAnimate::addTwinAnimate(
  std::shared_ptr<LayoutNode> nodeFrom,
  std::shared_ptr<LayoutNode> nodeTo)
{
  typedef std::shared_ptr<Domain::Element> TElement;

  auto attrBridge = getAttrBridge();
  bool isOnlyUpdatePaint = getIsOnlyUpdatePaint();

  std::list<TElement> noteToDirectChildrenSorted;
  do
  {
    if (!nodeTo)
    {
      break;
    }

    auto element = nodeTo->elementNode();
    if (!element)
    {
      break;
    }

    auto children = element->children();
    auto back_insert = std::back_inserter(noteToDirectChildrenSorted);

    std::copy_if(
      children.begin(),
      children.end(),
      back_insert,
      [](TElement element) { return static_cast<bool>(element); });

    noteToDirectChildrenSorted.sort([](TElement item0, TElement item1)
                                    { return item0->name() < item1->name(); });

  } while (false);

  ReplaceNodeAnimate::TTwins twins;
  if (nodeFrom)
  {
    auto element = nodeFrom->elementNode();
    if (element)
    {
      auto children = element->children();
      for (auto child : children)
      {
        if (!child)
        {
          continue;
        }

        auto it = std::lower_bound(
          noteToDirectChildrenSorted.begin(),
          noteToDirectChildrenSorted.end(),
          child,
          [](TElement element, TElement target)
          {
            assert(element && target);
            return element->name() < target->name();
          });

        if (it != noteToDirectChildrenSorted.end() && it->get()->name() == child->name())
        {
          twins.emplace(std::make_shared<LayoutNode>(child), std::make_shared<LayoutNode>(*it));
          noteToDirectChildrenSorted.erase(it);
        }
      }
    }
  }

  addTwinMatrixAnimate(twins);

  for (auto& item : twins)
  {
    // TODO should judge item.second? test it.
    if (ReplaceNodeAnimate::isContainerType(item.first->elementNode()))
    {
      addTwinAnimate(item.first, item.second);
    }
  }
}

MoveAnimate::MoveAnimate(
  milliseconds                  duration,
  milliseconds                  interval,
  std::shared_ptr<Interpolator> interpolator,
  std::shared_ptr<AttrBridge>   attrBridge,
  MoveType                      moveType,
  bool                          isSmart,
  MoveDirection                 moveDirection)
  : ReplaceNodeAnimate(duration, interval, interpolator, attrBridge)
  , m_moveType(moveType)
  , m_isSmart(isSmart)
  , m_moveDirection(moveDirection)
  , m_fromLTRB{}
  , m_ToLTRB{}
{
}

void MoveAnimate::start()
{
  auto attrBridge = getAttrBridge();
  auto from = getFrom();
  auto to = getTo();
  auto isOnlyUpdatePaint = getIsOnlyUpdatePaint();

  if (
    !attrBridge || !from || !to || !ReplaceNodeAnimate::isContainerType(from->elementNode()) ||
    !ReplaceNodeAnimate::isContainerType(to->elementNode()))
  {
    assert(false);
    return;
  }

  auto paintNodeFrom = attrBridge->getPaintNode(from);
  auto paintNodeTo = attrBridge->getPaintNode(to);
  if (!paintNodeFrom || !paintNodeTo)
  {
    assert(false);
    return;
  }

  m_fromLTRB = TransformHelper::getLTRB(
    *attrBridge->getWidth(paintNodeFrom),
    *attrBridge->getHeight(paintNodeFrom),
    *attrBridge->getMatrix(paintNodeFrom));

  auto widthTo = *attrBridge->getWidth(paintNodeTo);
  auto heightTo = *attrBridge->getHeight(paintNodeTo);
  auto matirxTo = *attrBridge->getMatrix(paintNodeTo);

  // TODO all ReplaceAnimate, should guarantee by user. Notify to gh.
  // TODO after gh do it, this place do not need moveToWindowTopLeft, just use matirxTo.
  auto to2TopLeftMatrix = TransformHelper::moveToWindowTopLeft(widthTo, heightTo, matirxTo);
  m_ToLTRB = TransformHelper::getLTRB(widthTo, heightTo, to2TopLeftMatrix);

  if (!m_isSmart)
  {
    auto matrixStart = getStartTranslateMatrix(to2TopLeftMatrix);
    auto matrixStop = getStopTranslateMatrix(to2TopLeftMatrix);

    attrBridge->updateMatrix(to, paintNodeTo, matrixStart, isOnlyUpdatePaint, {}, true);
    attrBridge->updateVisible(to, paintNodeTo, true, isOnlyUpdatePaint);

    auto animate = createAndAddNumberAnimate();
    animate->addTriggeredCallback(
      [](const std::vector<double>& value)
      {
        // TODO change From size to To.
        // TODO bigger To size, because need clip.
        // TODO those thing to smart? need think.
      });
    attrBridge->updateMatrix(to, paintNodeTo, matrixStop, isOnlyUpdatePaint, animate, true);
  }
  else
  {
    addStyleOpacityAnimate(from, paintNodeFrom, false, false);
    addStyleOpacityAnimate(to, paintNodeTo, true, false);

    // TODO need change nodeTo size. what about smartAnimate? maybe write this code to
    // replaceAnimate.

    // TODO after gh deal, maybe do not need this code.
    attrBridge->updateMatrix(to, paintNodeTo, to2TopLeftMatrix, isOnlyUpdatePaint, {}, true);

    attrBridge->updateVisible(to, paintNodeTo, true, isOnlyUpdatePaint);

    // auto originFromChild = paintNodeFrom->children();
    // auto originToChild = paintNodeTo->children();

    dealChildren(from, to, paintNodeFrom, paintNodeTo);
  }

  // TODO maybe let this code to replacenode
  addCallBackWhenStop(
    [from, paintNodeFrom, attrBridge, isOnlyUpdatePaint]()
    { attrBridge->updateVisible(from, paintNodeFrom, false, isOnlyUpdatePaint); });
}

void MoveAnimate::dealChildren(
  std::shared_ptr<LayoutNode> nodeFrom,
  std::shared_ptr<LayoutNode> nodeTo,
  layer::PaintNode*           paintNodeFrom,
  layer::PaintNode*           paintNodeTo)
{
  auto attrBridge = getAttrBridge();
  bool isOnlyUpdatePaint = getIsOnlyUpdatePaint();

  if (!paintNodeFrom || !paintNodeTo || !nodeFrom->elementNode() || !nodeTo->elementNode())
  {
    assert(false);
    return;
  }

  assert(
    ReplaceNodeAnimate::isContainerType(nodeFrom->elementNode()) &&
    ReplaceNodeAnimate::isContainerType(nodeTo->elementNode()));

  const auto& originFromChild = paintNodeFrom->children();
  const auto& originToChild = paintNodeTo->children();

  addCallBackWhenStop(
    [originFromChild, originToChild, paintNodeFrom, paintNodeTo]()
    {
      setPaintNodeChildren(paintNodeFrom, originFromChild);
      setPaintNodeChildren(paintNodeTo, originToChild);
    });

  std::unordered_map<std::string, std::shared_ptr<LayoutNode>> nodeIds;
  for (auto& item : nodeFrom->children())
  {
    nodeIds.emplace(item->id(), item);
  }
  for (auto& item : nodeTo->children())
  {
    nodeIds.emplace(item->id(), item);
  }
  assert(nodeIds.size() == nodeFrom->children().size() + nodeTo->children().size());

#ifdef DEBUG
  std::unordered_map<std::string, layer::PaintNodePtr> paintNodeIds;

  for (auto& item : originFromChild)
  {
    paintNodeIds.emplace(item->guid(), item);
  }
  for (auto& item : originToChild)
  {
    paintNodeIds.emplace(item->guid(), item);
  }
  assert(paintNodeIds.size() == originFromChild.size() + originToChild.size());
  assert(paintNodeIds.size() == nodeIds.size());
#endif // DEBUG

  auto addTranslateAnimate =
    [this, attrBridge, &nodeIds, isOnlyUpdatePaint](layer::PaintNode* paintNode)
  {
    if (!paintNode)
    {
      assert(false);
      return;
    }

    auto it = nodeIds.find(paintNode->guid());
    if (it == nodeIds.end() || !it->second->elementNode())
    {
      assert(false);
      return;
    }

    // TODO this place maybe wrong, need think.
    // TODO discuss with yl, our action is fine?
    auto& node = it->second;
    auto  matrix = *attrBridge->getMatrix(paintNode);
    auto  matrixStart = getStartTranslateMatrix(matrix);
    auto  matrixStop = getStopTranslateMatrix(matrix);
    bool  isContainer = ReplaceNodeAnimate::isContainerType(node->elementNode());
    auto  originMatrix = *attrBridge->getMatrix(paintNode);
    auto  animate = createAndAddNumberAnimate();

    attrBridge->updateMatrix(node, paintNode, matrixStart, isOnlyUpdatePaint, {}, isContainer);
    attrBridge->updateMatrix(node, paintNode, matrixStop, isOnlyUpdatePaint, animate, isContainer);
    animate->addCallBackWhenStop(
      [attrBridge, node, paintNode, originMatrix, isOnlyUpdatePaint, isContainer]() {
        attrBridge->updateMatrix(node, paintNode, originMatrix, isOnlyUpdatePaint, {}, isContainer);
      });
  };

  // <from, to>
  ReplaceNodeAnimate::TTwins twins;

  std::list<layer::PaintNodePtr> childrenInAnimate(originToChild.begin(), originToChild.end());
  auto                           itSearch = childrenInAnimate.begin();
  auto                           itInsert = itSearch;

  for (auto it = originFromChild.begin(); it != originFromChild.end(); ++it)
  {
    if (!(*it) || !*attrBridge->getVisible(it->get()))
    {
      continue;
    }

    auto result = std::find_if(
      itSearch,
      childrenInAnimate.end(),
      [it, attrBridge](layer::PaintNodePtr obj)
      {
        if (!obj || !*attrBridge->getVisible(obj.get()))
        {
          return false;
        }

        return (*it)->name() == obj->name();
      });

    if (result == childrenInAnimate.end())
    {
      childrenInAnimate.insert(itInsert, *it);
    }
    else
    {
      for (auto tmp = itSearch; tmp != result; ++tmp)
      {
        addTranslateAnimate(*tmp);
      }

      childrenInAnimate.insert(result, *it);

      auto itFrom = nodeIds.find(it->get()->guid());
      auto itTo = nodeIds.find(result->get()->guid());
      assert(itFrom != nodeIds.end() && itTo != nodeIds.end());
      if (itFrom != nodeIds.end() && itTo != nodeIds.end())
      {
        twins.emplace(itFrom->second, itTo->second);

        auto& childFrom = itFrom->second;
        auto& childTo = itTo->second;
        auto  elementFrom = childFrom->elementNode();
        auto  elementTo = childTo->elementNode();
        auto  childPaintFrom = attrBridge->getPaintNode(childFrom);
        auto  childPaintTo = attrBridge->getPaintNode(childTo);
        assert(elementFrom && elementTo);

        if (
          ReplaceNodeAnimate::isContainerType(elementFrom) &&
          ReplaceNodeAnimate::isContainerType(elementTo))
        {
          dealChildren(childFrom, childTo, childPaintFrom, childPaintTo);
        }
        else
        {
          // TODO
        }

        addStyleOpacityAnimate(childFrom, childPaintFrom, false, false);
        addStyleOpacityAnimate(childTo, childPaintTo, true, false);
      }

      itSearch = ++result;
      itInsert = itSearch;
    }
  }

  for (auto it = itSearch; it != childrenInAnimate.end(); ++it)
  {
    addTranslateAnimate(*it);
  }

  clearPaintNodeChildren(paintNodeFrom);
  clearPaintNodeChildren(paintNodeTo);
  for (auto& item : childrenInAnimate)
  {
    paintNodeTo->addChild(item);
  }

  addTwinMatrixAnimate(twins);
}

std::array<double, 6> MoveAnimate::getStartTranslateMatrix(const std::array<double, 6>& selfMatrix)
{
  if (m_moveType == MOVE_IN)
  {
    switch (m_moveDirection)
    {
      case VGG::MoveAnimate::FROM_RIGHT:
        return TransformHelper::translate(m_fromLTRB[2], m_fromLTRB[1], selfMatrix);
      case VGG::MoveAnimate::FROM_UP:
        return TransformHelper::translate(m_ToLTRB[0], -m_ToLTRB[3], selfMatrix);
      case VGG::MoveAnimate::FROM_LEFT:
        return TransformHelper::translate(-m_ToLTRB[2], m_ToLTRB[0], selfMatrix);
      case VGG::MoveAnimate::FROM_BOTTOM:
        return TransformHelper::translate(m_fromLTRB[0], m_fromLTRB[3], selfMatrix);
      default:
        break;
    }
  }

  // TODO not complete for MOVE_OUT
  assert(false);
  return {};
}

std::array<double, 6> MoveAnimate::getStopTranslateMatrix(const std::array<double, 6>& selfMatrix)
{
  if (m_moveType == MOVE_IN)
  {
    switch (m_moveDirection)
    {
      case VGG::MoveAnimate::FROM_RIGHT:
        return TransformHelper::translate(m_fromLTRB[0], m_fromLTRB[1], selfMatrix);
      case VGG::MoveAnimate::FROM_UP:
        return TransformHelper::translate(m_ToLTRB[0], m_ToLTRB[0], selfMatrix);
      case VGG::MoveAnimate::FROM_LEFT:
        return TransformHelper::translate(m_ToLTRB[0], m_ToLTRB[0], selfMatrix);
      case VGG::MoveAnimate::FROM_BOTTOM:
        return TransformHelper::translate(m_fromLTRB[0], m_fromLTRB[0], selfMatrix);
      default:
        break;
    }
  }

  // TODO not complete for MOVE_OUT
  assert(false);
  return {};
}

// TODO for replace animate, an container match an not container, need test it.
// TODO test back for all replace animate