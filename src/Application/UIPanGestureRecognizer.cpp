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

#include "Utility/VggDate.hpp"

using namespace VGG::UIKit;

void UIPanGestureRecognizer::touchesBegan(UEvent e)
{
}

void UIPanGestureRecognizer::touchesMoved(UEvent e)
{
  Point translation;
  if (e.type == VGG_MOUSEMOTION)
  {
    translation.x = e.motion.xrel;
    translation.y = e.motion.yrel;
  }
  else if (e.type == VGG_TOUCHMOTION)
  {
    // todo
    // auto clientSize = m_presenter->viewSize();
    // return handleTranslate(evt.xrel * clientSize.width, evt.yrel * clientSize.height);
  }
  else
  {
    return;
  }

  if (m_state == EUIGestureRecognizerState::POSSIBLE)
  {
    m_lastMovementTime = nowTimestampInSeconds();
    setTranslation(translation);
    m_state = EUIGestureRecognizerState::BEGAN;
  }
  else if (translate(translation))
  {
    m_state = EUIGestureRecognizerState::CHANGED;
  }
}

void UIPanGestureRecognizer::touchesEnded(UEvent e)
{
  translate(Point::zero());
  m_state = EUIGestureRecognizerState::ENDED;
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
    m_lastMovementTime = now;
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