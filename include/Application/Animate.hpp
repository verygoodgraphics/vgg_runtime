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
#include <memory>
#include <chrono>
#include <functional>
#include <cassert>
#include <vector>

namespace VGG
{

class Timer;
class LayoutNode;
class AttrBridge;
using std::chrono::milliseconds;

class Interpolator
{
public:
  virtual ~Interpolator();

public:
  virtual double operator()(
    milliseconds passedTime,
    double       from,
    double       to,
    milliseconds duration) = 0;

  auto isFinished();

protected:
  void setFinished();

private:
  bool m_finished{ false };
};

class LinearInterpolator : public Interpolator
{
public:
  virtual double operator()(milliseconds passedTime, double from, double to, milliseconds duration)
    override;
};

class Animate
{
public:
  Animate(milliseconds duration, milliseconds interval, std::shared_ptr<Interpolator> interpolator);
  virtual ~Animate();

public:
  void stop();
  bool isRunning();
  bool isFinished();

  auto getDuration();
  auto getInterval();
  auto getInterpolator();
  auto getStartTime();

  void addCallBackWhenStop(std::function<void()>&& fun);

protected:
  virtual void            start() = 0;
  std::shared_ptr<Timer>& getTimerRef();

private:
  milliseconds                          m_duration;
  milliseconds                          m_interval;
  std::shared_ptr<Interpolator>         m_interpolator;
  std::shared_ptr<Timer>                m_timer;
  std::chrono::steady_clock::time_point m_startTime;
  std::vector<std::function<void()>>    m_callbackWhenStop;
};

class AnimateManage
{
  friend AttrBridge;

public:
  void deleteFinishedAnimate();
  bool hasRunningAnimation() const;

private:
  void addAnimate(std::shared_ptr<Animate> animate);

private:
  std::vector<std::shared_ptr<Animate>> m_animates;
};

class NumberAnimate : public Animate
{
  friend AttrBridge;

  typedef std::vector<double> TParam;

public:
  NumberAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator);

private:
  virtual void start() override;
  void         setFromTo(const TParam& from, const TParam& to);
  void         setAction(std::function<void(const TParam&)> action);

private:
  TParam                             m_from;
  TParam                             m_to;
  TParam                             m_nowValue;
  std::function<void(const TParam&)> m_action;
};

class ReplaceNodeAnimate : public Animate
{
  friend AttrBridge;

public:
  ReplaceNodeAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator,
    std::shared_ptr<AttrBridge>   attrBridge);

private:
  void setFromTo(std::shared_ptr<LayoutNode> from, std::shared_ptr<LayoutNode> to);
  void setIsOnlyUpdatePaint(bool isOnlyUpdatePaint);

protected:
  auto getFrom();
  auto getTo();
  auto getAttrBridge();
  auto getIsOnlyUpdatePaint();

private:
  std::shared_ptr<LayoutNode> m_from;
  std::shared_ptr<LayoutNode> m_to;
  std::shared_ptr<AttrBridge> m_attrBridge;
  bool                        m_isOnlyUpdatePaint;
};

class DissolveAnimate : public ReplaceNodeAnimate
{
public:
  DissolveAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator,
    std::shared_ptr<AttrBridge>   attrBridge);

private:
  virtual void start() override;

private:
  std::shared_ptr<NumberAnimate> m_fromAnimate;
  std::shared_ptr<NumberAnimate> m_toAnimate;
};

// TODO
class SmartAnimate : public ReplaceNodeAnimate
{
public:
  SmartAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator,
    std::shared_ptr<AttrBridge>   attrBridge);

private:
  virtual void start() override;
  void         correlateNode();

private:
  std::vector<NumberAnimate> m_animates;

  // true: by id, false: by name chain
  bool m_correlateById;
};

} // namespace VGG
