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
  if (isRunning())
  {
    m_timer->invalidate();
    m_timer = nullptr;
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

void AnimateManage::addAnimate(std::shared_ptr<Animate> animate)
{
  m_animates.emplace_back(animate);
}

void AnimateManage::deleteFinishedAnimate()
{
  auto it = std::remove_if(
    m_animates.begin(),
    m_animates.end(),
    [](auto animate) { return animate->isFinished(); });

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

      const auto size = m_nowValue.size();
      for (size_t i = 0; i < size; ++i)
      {
        m_nowValue[i] = (*interpolator)(
          std::chrono::duration_cast<milliseconds>(
            std::chrono::steady_clock::now() - getStartTime()),
          m_from.at(i),
          m_to.at(i),
          getDuration());
      }

      m_action(m_nowValue);
    },
    true);
}

void NumberAnimate::setFromTo(const std::vector<double>& from, const std::vector<double>& to)
{
  assert(!from.empty() && from.size() == to.size());
  m_from = from;
  m_to = to;
  m_nowValue.resize(from.size());
}

void NumberAnimate::setAction(std::function<void(const std::vector<double>&)> action)
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
  assert(from && to && attrBridge);

  auto createNumberAnimate = [this]()
  { return std::make_shared<NumberAnimate>(getDuration(), getInterval(), getInterpolator()); };

  auto opacity = attrBridge->getNodeOpacity(to, false);
  assert(opacity);
  attrBridge->setNodeOpacity(to, 0, false);
  attrBridge->setNodeOpacity(to, 0, true);

  attrBridge
    ->updateOpacity(to, opacity ? *opacity : 0, getIsOnlyUpdatePaint(), createNumberAnimate());
  attrBridge->updateOpacity(from, 0, getIsOnlyUpdatePaint(), createNumberAnimate());
}
