/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "UIScrollViewAnimationDeceleration.hpp"

#include "UIScrollView.hpp"

#include "Utility/Log.hpp"
#include "Utility/VggDate.hpp"

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG::UIKit;

namespace
{
static const float  minimumBounceVelocityBeforeReturning = 100;
static const double returnAnimationDuration = 0.33;
static const double physicsTimeStep = 1 / 120.;
static const float  springTightness = 7;
static const float  springDampening = 15;

float uiLinearInterpolation(float t, float start, float end)
{
  if (t <= 0)
  {
    return start;
  }
  else if (t >= 1)
  {
    return end;
  }
  else
  {
    return t * end + (1 - t) * start;
  }
}

float uiQuadraticEaseOut(float t, float start, float end)
{
  if (t <= 0)
  {
    return start;
  }
  else if (t >= 1)
  {
    return end;
  }
  else
  {
    return uiLinearInterpolation(2 * t - t * t, start, end);
  }
}

static float clamp(float v, float min, float max)
{
  return (v < min) ? min : (v > max) ? max : v;
}

static float clampedVelocty(float v)
{
  DEBUG("clampedVelocty, v = %f", v);
  const float V = 200000;
  return clamp(v, -V, V);
}

static float spring(
  float velocity,
  float position,
  float restPosition,
  float tightness,
  float dampening)
{
  const float d = position - restPosition;
  return (-tightness * d) - (dampening * velocity);
}

static bool bounceComponent(double t, UIScrollViewAnimationDecelerationComponent* c, float to)
{
  if (c->bounced && c->returnTime != 0)
  {
    DEBUG("bounceComponent, 1: returnTime = %f", c->returnTime);
    const double returnBounceTime = std::min(1., ((t - c->returnTime) / returnAnimationDuration));
    c->position = uiQuadraticEaseOut(returnBounceTime, c->returnFrom, to);
    DEBUG(
      "bounceComponent, 1: position = %f, returnBounceTime = %f, from = %f, to = %f",
      c->position,
      returnBounceTime,
      c->returnFrom,
      to);
    return (returnBounceTime == 1);
  }
  else if (c->velocity != 0)
  {
    DEBUG(
      "bounceComponent, 2: to = %f, position = %f, velocity = %f",
      to,
      c->position,
      c->velocity);
    const float F = spring(c->velocity, c->position, to, springTightness, springDampening);

    c->velocity += F * physicsTimeStep;
    c->position += c->velocity * physicsTimeStep;

    c->bounced = true;

    if (fabsf(c->velocity) < minimumBounceVelocityBeforeReturning)
    {
      c->returnFrom = c->position;
      c->returnTime = t;
      DEBUG(
        "bounceComponent, 2: set returnFrom = %f, returnTime = %f",
        c->returnFrom,
        c->returnTime);
    }

    return false;
  }
  else
  {
    DEBUG("bounceComponent, 3: no bounce");
    return true;
  }
}

} // namespace

UIScrollViewAnimationDeceleration::UIScrollViewAnimationDeceleration(UIScrollView* sv, Point v)
  : UIScrollViewAnimation(sv)
{
  m_lastMomentumTime = beginTime();

  m_x.decelerateTime = beginTime();
  m_x.velocity = clampedVelocty(v.x);
  m_x.position = m_scrollView->contentOffset().x;
  m_x.returnFrom = 0;
  m_x.returnTime = 0;
  m_x.bounced = false;

  m_y.decelerateTime = beginTime();
  m_y.velocity = clampedVelocty(v.y);
  m_y.position = m_scrollView->contentOffset().y;
  m_y.returnFrom = 0;
  m_x.returnTime = 0;
  m_y.bounced = false;

  if (m_x.velocity == 0)
  {
    m_x.bounced = true;
    m_x.returnTime = beginTime();
    m_x.returnFrom = m_x.position;
  }

  if (m_y.velocity == 0)
  {
    m_y.bounced = true;
    m_y.returnTime = beginTime();
    m_y.returnFrom = m_y.position;
  }
}

bool UIScrollViewAnimationDeceleration::animate()
{
  const auto currentTime = nowTimestampInSeconds();
  DEBUG("UIScrollViewAnimationDeceleration::animate: currentTime = %f", currentTime);

  auto timeDiff = currentTime - m_lastMomentumTime;

  const auto isFinishedWaitingForMomentumScroll = (timeDiff > 0.15f);

  auto finished = false;

  while (!finished && currentTime >= beginTime())
  {
    auto confinedOffset = m_scrollView->confinedContentOffset(Point{ m_x.position, m_y.position });

    const auto verticalIsFinished = bounceComponent(beginTime(), &m_y, confinedOffset.y);
    const auto horizontalIsFinished = bounceComponent(beginTime(), &m_x, confinedOffset.x);

    finished = (verticalIsFinished && horizontalIsFinished && isFinishedWaitingForMomentumScroll);

    m_beginTime += physicsTimeStep;
  }

  m_scrollView->setRestrainedContentOffset(Point{ m_x.position, m_y.position });

  return finished;
}

void UIScrollViewAnimationDeceleration::momentumScrollBy(Point delta)
{
  m_lastMomentumTime = nowTimestampInSeconds();

  if (!m_x.bounced)
  {
    m_x.position += delta.x;
    m_x.velocity = clampedVelocty(delta.x / (m_lastMomentumTime - m_x.decelerateTime));
    m_x.decelerateTime = m_lastMomentumTime;
  }

  if (!m_y.bounced)
  {
    m_y.position += delta.y;
    m_y.velocity = clampedVelocty(delta.y / (m_lastMomentumTime - m_y.decelerateTime));
    m_y.decelerateTime = m_lastMomentumTime;
  }
}