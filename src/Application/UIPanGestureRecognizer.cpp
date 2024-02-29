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

#include "UIPanGestureRecognizer.hpp"

#include "Application/UIView.hpp"

#include "Utility/Log.hpp"
#include "Utility/VggDate.hpp"

#undef DEBUG
#define DEBUG(msg, ...)

using namespace VGG::UIKit;

void UIPanGestureRecognizer::touchesBegan(UEvent e)
{
  setState(EUIGestureRecognizerState::POSSIBLE);
}

void UIPanGestureRecognizer::touchesMoved(UEvent e)
{
  Point translation;
  if (e.type == VGG_MOUSEMOTION)
  {
    translation.x = e.motion.xrel;
    translation.y = e.motion.yrel;
    DEBUG(
      "UIPanGestureRecognizer::touchesMoved: mouse motion, x = %f, y = %f",
      translation.x,
      translation.y);
  }
  else if (e.type == VGG_TOUCHMOTION)
  {
    DEBUG(
      "UIPanGestureRecognizer::touchesMoved: touch motion, x = %f, y = %f",
      e.touch.xrel,
      e.touch.yrel);
    if (m_view)
    {
      auto clientSize = m_view->size();
      translation.x = e.touch.xrel * clientSize.width;
      translation.y = e.touch.yrel * clientSize.height;
      DEBUG(
        "UIPanGestureRecognizer::touchesMoved: translation: x = %f, y = %f",
        translation.x,
        translation.y);
    }
    else
    {
      return;
    }
  }
  else
  {
    return;
  }

  if (state() == EUIGestureRecognizerState::POSSIBLE)
  {
    DEBUG("UIPanGestureRecognizer::touchesMoved: state -> began");
    m_lastMovementTime = nowTimestampInSeconds();
    setTranslation(translation);
    setState(EUIGestureRecognizerState::BEGAN);
  }
  else if (
    state() == EUIGestureRecognizerState::BEGAN || state() == EUIGestureRecognizerState::CHANGED)
  {
    if (translate(translation))
    {
      DEBUG("UIPanGestureRecognizer::touchesMoved: state -> changed");
      setState(EUIGestureRecognizerState::CHANGED);
    }
  }
  else
  {
    DEBUG("UIPanGestureRecognizer::touchesMoved: state is %d", (int)state());
  }
}

void UIPanGestureRecognizer::touchesEnded(UEvent e)
{
  translate(m_lastDelta); // no more movement, use the last delta to calculate velocity
  setState(EUIGestureRecognizerState::ENDED);
}

void UIPanGestureRecognizer::setTranslation(Point translation)
{
  m_velocity = Point::zero();
  m_translation = translation;
}

bool UIPanGestureRecognizer::translate(Point delta)
{
  auto now = nowTimestampInSeconds();
  auto timeDiff = now - m_lastMovementTime;

  if ((delta != Point::zero()) && timeDiff > 0)
  {
    m_translation.x += delta.x;
    m_translation.y += delta.y;
    m_velocity.x = delta.x / timeDiff;
    m_velocity.y = delta.y / timeDiff;
    DEBUG(
      "UIPanGestureRecognizer::translate: vecocity: x = %f, y = %f",
      m_velocity.x,
      m_velocity.y);
    m_lastMovementTime = now;
    m_lastDelta = delta;
    return true;
  }
  else
  {
    return false;
  }
}

Point UIPanGestureRecognizer::translation()
{
  return m_translation;
}

Point UIPanGestureRecognizer::velocity()
{
  return m_velocity;
}

void UIPanGestureRecognizer::setState(EUIGestureRecognizerState state)
{
  UIGestureRecognizer::setState(state);
  if (m_handler)
  {
    m_handler(*this);
  }
}