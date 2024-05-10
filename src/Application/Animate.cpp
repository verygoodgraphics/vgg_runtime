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
#include <unordered_map>

using namespace VGG;

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

void ReplaceNodeAnimate::addChildAnimate(std::shared_ptr<Animate> animate)
{
  m_childAnimates.emplace_back(animate);
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

  auto createNumberAnimate = [this]()
  { return std::make_shared<NumberAnimate>(getDuration(), getInterval(), getInterpolator()); };

  if (to)
  {
    auto animate = createNumberAnimate();
    addChildAnimate(animate);

    auto paintNodeTo = attrBridge->getPaintNode(to);
    auto opacity = AttrBridge::getOpacity(paintNodeTo);
    assert(opacity);
    attrBridge->updateOpacity(to, paintNodeTo, 0, isOnlyUpdatePaint);
    attrBridge->updateVisible(to, paintNodeTo, true, isOnlyUpdatePaint);
    attrBridge->updateOpacity(to, paintNodeTo, opacity ? *opacity : 1, isOnlyUpdatePaint, animate);
  }

  if (from)
  {
    auto animate = createNumberAnimate();
    addChildAnimate(animate);

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

  addCallBackWhenStop(
    [attrBridge, from, isOnlyUpdatePaint]()
    { attrBridge->updateVisible(from, attrBridge->getPaintNode(from), false, isOnlyUpdatePaint); });

  addTwinAnimate(from, to);
  addOpacityAnimate(from, false);
  addOpacityAnimate(to, true);
  attrBridge->updateVisible(to, attrBridge->getPaintNode(to), true, isOnlyUpdatePaint);
}

void SmartAnimate::addOpacityAnimate(std::shared_ptr<LayoutNode> node, bool toVisible)
{
  if (!node || !node->elementNode())
  {
    return;
  }

  auto attrBridge = getAttrBridge();
  auto isOnlyUpdatePaint = getIsOnlyUpdatePaint();
  auto paintNode = attrBridge->getPaintNode(node);

  // for fill
  do
  {
    auto size = attrBridge->getFillSize(paintNode);

    if (!size)
    {
      assert(false);
      break;
    }

    for (size_t i = 0; i < *size; ++i)
    {
      auto opacity = AttrBridge::getFillOpacity(paintNode, i);
      if (!opacity)
      {
        assert(false);
        continue;
      }

      auto animate =
        std::make_shared<NumberAnimate>(getDuration(), getInterval(), getInterpolator());
      addChildAnimate(animate);

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

  if (ReplaceNodeAnimate::isContainerType(node->elementNode()))
  {
    auto& children = node->children();
    for (auto child : children)
    {
      addOpacityAnimate(child->shared_from_this(), toVisible);
    }
  }
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

  std::unordered_map<std::shared_ptr<LayoutNode>, std::shared_ptr<LayoutNode>> twins;
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

  for (auto item : twins)
  {
    auto itemFrom = item.first;
    auto itemTo = item.second;

    auto objectFrom = AttrBridge::getlayoutNodeObject(itemFrom);
    auto objectTo = AttrBridge::getlayoutNodeObject(itemTo);
    if (!objectFrom || !objectTo)
    {
      continue;
    }

    if (objectTo->matrix.size() != 6)
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

    auto animate = std::make_shared<NumberAnimate>(getDuration(), getInterval(), getInterpolator());
    addChildAnimate(animate);

    animate->addTriggeredCallback(std::bind(
      AttrBridge::setTwinMatrix,
      itemFrom,
      itemTo,
      paintNodeTo,
      std::placeholders::_1,
      isOnlyUpdatePaint));

    auto paintNodeFromOriginalMatrix = attrBridge->getMatrix(paintNodeFrom);
    bool itemFromIsContainer = ReplaceNodeAnimate::isContainerType(itemFrom->elementNode());

    animate->addCallBackWhenStop(
      [paintNodeFromOriginalMatrix,
       itemFrom,
       paintNodeFrom,
       attrBridge,
       isOnlyUpdatePaint,
       itemFromIsContainer]()
      {
        assert(paintNodeFromOriginalMatrix);
        if (paintNodeFromOriginalMatrix)
        {
          attrBridge->updateMatrix(
            itemFrom,
            paintNodeFrom,
            *paintNodeFromOriginalMatrix,
            isOnlyUpdatePaint,
            {},
            itemFromIsContainer);
        }
      });

    attrBridge->updateMatrix(
      itemFrom,
      paintNodeFrom,
      transform,
      isOnlyUpdatePaint,
      animate,
      itemFromIsContainer);
  }

  for (auto item : twins)
  {
    if (ReplaceNodeAnimate::isContainerType(item.first->elementNode()))
    {
      addTwinAnimate(item.first, item.second);
    }
  }
}
