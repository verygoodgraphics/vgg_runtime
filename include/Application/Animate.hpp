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
  void         stop();
  bool         isRunning();
  virtual bool isFinished() = 0;

  auto getDuration();
  auto getInterval();
  auto getInterpolator();
  auto getStartTime();

protected:
  virtual void            start() = 0;
  std::shared_ptr<Timer>& getTimerRef();

private:
  milliseconds                          m_duration;
  milliseconds                          m_interval;
  std::shared_ptr<Interpolator>         m_interpolator;
  std::shared_ptr<Timer>                m_timer;
  std::chrono::steady_clock::time_point m_startTime;
};

class AnimateManage
{
  friend AttrBridge;

public:
  void deleteFinishedAnimate();

private:
  void addAnimate(std::shared_ptr<Animate> animate);

private:
  std::vector<std::shared_ptr<Animate>> m_animates;
};

class NumberAnimate : public Animate
{
  friend AttrBridge;

public:
  NumberAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator);

public:
  virtual bool isFinished() override;

protected:
  virtual void start() override;
  void         setFromTo(const std::vector<double>& from, const std::vector<double>& to);
  void         setAction(std::function<void(const std::vector<double>&)> action);

private:
  std::vector<double>                             m_from;
  std::vector<double>                             m_to;
  std::vector<double>                             m_nowValue;
  std::function<void(const std::vector<double>&)> m_action;
};

// class DissolveAnimate : public Animate
// {
// };

// class SmartAnimate : public Animate
// {
// };

} // namespace VGG
