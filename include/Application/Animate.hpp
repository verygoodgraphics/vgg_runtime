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
#include <array>
#include <optional>

namespace VGG
{

class Timer;
class LayoutNode;
class AttrBridge;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

namespace Domain
{
class Element;
}

namespace layer
{
class PaintNode;
}

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
  Animate(Animate* parent);
  virtual ~Animate();

public:
  bool                          isIndependent();
  bool                          isRunning();
  bool                          isFinished();
  milliseconds                  getDuration();
  milliseconds                  getInterval();
  std::shared_ptr<Interpolator> getInterpolator();

  // The calling order of the added functions is the reverse of the order in which they were added.
  void addCallBackWhenStop(std::function<void()>&& fun);
  void stop();

public:
  virtual void timerCallback() = 0;

protected:
  virtual void start();

  steady_clock::time_point getLastTriggeredTime();
  steady_clock::time_point getStartTime();
  milliseconds             getPassedTime();
  Animate*                 getParent();
  void                     addChildAnimate(std::shared_ptr<Animate> child);
  const auto&              getChildAnimate();

private:
  std::optional<milliseconds>             m_duration;
  std::optional<milliseconds>             m_interval;
  std::shared_ptr<Interpolator>           m_interpolator;
  Animate*                                m_parent;
  std::vector<std::shared_ptr<Animate>>   m_childAnimates;
  std::shared_ptr<Timer>                  m_timer;
  std::optional<steady_clock::time_point> m_startTime;
  std::optional<steady_clock::time_point> m_lastTriggeredTime;
  std::vector<std::function<void()>>      m_callbackWhenStop;
};

class AnimateManage
{
  friend AttrBridge;

public:
  void deleteFinishedAnimate();
  bool hasRunningAnimation() const;

private:
  // if animate is not isIndependent, then do nothing.
  void addAnimate(std::shared_ptr<Animate> animate);

private:
  std::vector<std::shared_ptr<Animate>> m_animates;
};

class NumberAnimate : public Animate
{
  friend AttrBridge;

  typedef std::vector<double>                TParam;
  typedef std::function<void(const TParam&)> TTriggeredCallback;

public:
  NumberAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator);

  NumberAnimate(Animate* parent);

public:
  void        addTriggeredCallback(TTriggeredCallback&& fun);
  const auto& getTriggeredCallback();

private:
  virtual void timerCallback() override;
  void         setFromTo(const TParam& from, const TParam& to);

private:
  TParam                          m_from;
  TParam                          m_to;
  TParam                          m_nowValue;
  std::vector<TTriggeredCallback> m_triggeredCallback;
};

// Note: Ensure that the properties of the old node, with the exception of a few specific
// properties, are restored after the animation ends.
class ReplaceNodeAnimate : public Animate
{
  friend AttrBridge;

public:
  typedef std::unordered_map<std::shared_ptr<LayoutNode>, std::shared_ptr<LayoutNode>> TTwins;
  typedef std::function<void()> TTriggeredCallback;

public:
  ReplaceNodeAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator,
    std::shared_ptr<AttrBridge>   attrBridge);

public:
  void        addTriggeredCallback(TTriggeredCallback&& fun);
  const auto& getTriggeredCallback();

public:
  static bool isContainerType(const Domain::Element* node);

private:
  virtual void timerCallback() override;
  void         setFromTo(std::shared_ptr<LayoutNode> from, std::shared_ptr<LayoutNode> to);
  void         setIsOnlyUpdatePaint(bool isOnlyUpdatePaint);

protected:
  auto getFrom();
  auto getTo();
  auto getAttrBridge();
  auto getIsOnlyUpdatePaint();

  // After animation finished, all modified properties will be reverted to their original values.
  void addStyleOpacityAnimate(
    std::shared_ptr<LayoutNode> node,
    layer::PaintNode*           paintNode,
    bool                        toVisible,
    bool                        recursive);

  // After animation finished, all modified properties will be reverted to their original values.
  void addTwinMatrixAnimate(const TTwins& twins);

  std::shared_ptr<NumberAnimate> createAndAddNumberAnimate();

private:
  std::shared_ptr<LayoutNode>     m_from;
  std::shared_ptr<LayoutNode>     m_to;
  std::shared_ptr<AttrBridge>     m_attrBridge;
  bool                            m_isOnlyUpdatePaint;
  std::vector<TTriggeredCallback> m_triggeredCallback;
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
};

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

  void addTwinAnimate(std::shared_ptr<LayoutNode> nodeFrom, std::shared_ptr<LayoutNode> nodeTo);
};

class MoveAnimate : public ReplaceNodeAnimate
{
public:
  enum MoveType
  {
    MOVE_IN,

    // TODO not complete.
    MOVE_OUT
  };

  enum MoveDirection
  {
    FROM_RIGHT,
    FROM_UP,
    FROM_LEFT,
    FROM_BOTTOM
  };

public:
  MoveAnimate(
    milliseconds                  duration,
    milliseconds                  interval,
    std::shared_ptr<Interpolator> interpolator,
    std::shared_ptr<AttrBridge>   attrBridge,
    MoveType                      moveType,
    bool                          isSmart,
    MoveDirection                 moveDirection);

private:
  virtual void start() override;

  // Note: When an object needs to update its coordinates in the global coordinate system using
  // updateMatrix, if the parent object is also updating its matrix within the same callback cycle,
  // the parent object's matrix must be updated first.
  void dealChildren(
    std::shared_ptr<LayoutNode> nodeFrom,
    std::shared_ptr<LayoutNode> nodeTo,
    layer::PaintNode*           paintNodeFrom,
    layer::PaintNode*           paintNodeTo);

  std::array<double, 6> getStartTranslateMatrix(const std::array<double, 6>& selfMatrix);
  std::array<double, 6> getStopTranslateMatrix(const std::array<double, 6>& selfMatrix);

private:
  MoveType      m_moveType;
  bool          m_isSmart;
  MoveDirection m_moveDirection;

  std::array<double, 4> m_fromLTRB;
  std::array<double, 4> m_ToLTRB;
};

} // namespace VGG
