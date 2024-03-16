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

#include "UIScrollView.hpp"

#include "UIScrollViewAnimationDeceleration.hpp"

#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

#include <algorithm>
#include <cmath>

#undef DEBUG
#define DEBUG(msg, ...)

#define VERBOSE DEBUG
#undef VERBOSE
#define VERBOSE(msg, ...)

using namespace VGG;
using namespace VGG::UIKit;

UIScrollView::UIScrollView()
  : UIView()
{
  m_panGestureRecognizer.setHandler([this](UIPanGestureRecognizer& pan)
                                    { this->handleGesture(pan); });
  m_panGestureRecognizer.setView(this);
}

bool UIScrollView::onEvent(UEvent event, void* userData)
{
  if (isScrollEnabled() && (canScrollHorizontal() || canScrollVertical()))
  {
    VERBOSE("UIScrollView::onEvent: receive event(0x%x)", event.type);

    bool handled = false;
    switch (event.type)
    {
#ifndef EMSCRIPTEN
        // handle touches only
        // in browsers, mouse and touch events are both received, first mouse then touch.
      case VGG_MOUSEBUTTONDOWN:
#endif
      case VGG_TOUCHDOWN:
        VERBOSE("UIScrollView::onEvent: touch down");
        m_panGestureRecognizer.touchesBegan(event);
        break;

#ifndef EMSCRIPTEN
      case VGG_MOUSEMOTION:
#endif
      case VGG_TOUCHMOTION:
        VERBOSE("UIScrollView::onEvent: touch move");
        m_panGestureRecognizer.touchesMoved(event);
        break;

#ifndef EMSCRIPTEN
      case VGG_MOUSEBUTTONUP:
#endif
      case VGG_TOUCHUP:
        VERBOSE("UIScrollView::onEvent: touch up");
        if (m_dragging)
        {
          handled = true;
        }
        m_panGestureRecognizer.touchesEnded(event);
        break;

      default:
        break;
    }

    switch (event.type)
    {
      case VGG_MOUSEBUTTONDOWN:
      case VGG_MOUSEMOTION:
      case VGG_MOUSEBUTTONUP:
      case VGG_TOUCHDOWN:
      case VGG_TOUCHMOTION:
      case VGG_TOUCHUP:
        if (handled || m_dragging)
        {
          VERBOSE("UIScrollView::onEvent: event(0x%x) is handled", event.type);
          return true;
        }
      default:
        break;
    }
  }

  return UIView::onEvent(event, userData);
}

void UIScrollView::setContentSize(Size size)
{
  m_contentSize = size;
}

void UIScrollView::setContentOffset(Point offset, bool byMouseWheel)
{
  VERBOSE("UIScrollView::setContentOffset: x = %f, y = %f", offset.x, offset.y);
  m_contentOffset = offset;
  setOffset(Offset{ offset.x, offset.y });

  if (byMouseWheel)
  {
    cancelScrollAnimation();
  }
}

bool UIScrollView::canScrollHorizontal()
{
  return m_contentSize.width > size().width;
}

bool UIScrollView::canScrollVertical()
{
  return m_contentSize.height > size().height;
}

void UIScrollView::handleGesture(UIKit::UIPanGestureRecognizer& recognizer)
{
  switch (recognizer.state())
  {
    case EUIGestureRecognizerState::BEGAN:
      VERBOSE("UIScrollView::handleGesture: began");
      beginDragging();
      break;

    case EUIGestureRecognizerState::CHANGED:
      VERBOSE("UIScrollView::handleGesture: changed");
      dragBy(m_panGestureRecognizer.translation());
      m_panGestureRecognizer.setTranslation(Point::zero());
      break;

    case EUIGestureRecognizerState::ENDED:
      VERBOSE("UIScrollView::handleGesture: ended");
      endDraggingWithDecelerationVelocity(m_panGestureRecognizer.velocity());
      break;

    default:
      break;
  }
}
void UIScrollView::beginDragging()
{
  if (!m_dragging)
  {
    m_dragging = true;

    cancelScrollAnimation();
  }
}

void UIScrollView::dragBy(Point delta)
{
  if (m_dragging)
  {
    const auto originalOffset = m_contentOffset;

    auto proposedOffset = originalOffset;
    proposedOffset.x += delta.x;
    proposedOffset.y += delta.y;

    const auto confinedOffset = confinedContentOffset(proposedOffset);

    if (m_bouncesHorizontally || m_bouncesVertically)
    {
      if (m_bouncesHorizontally)
      {
        // shouldHorizontalBounce
        if (!doublesNearlyEqual(proposedOffset.x, confinedOffset.x))
        {
          proposedOffset.x = originalOffset.x + (0.055 * delta.x);
        }
      }
      else
      {
        proposedOffset.x = confinedOffset.x;
      }

      if (m_bouncesVertically)
      {
        // shouldVerticalBounce
        if (!doublesNearlyEqual(proposedOffset.y, confinedOffset.y))
        {
          proposedOffset.y = originalOffset.y + (0.055 * delta.y);
        }
      }
      else
      {
        proposedOffset.y = confinedOffset.y;
      }

      setRestrainedContentOffset(proposedOffset);
    }
    else
    {
      setContentOffset(confinedOffset);
    }
  }
}

void UIScrollView::endDraggingWithDecelerationVelocity(Point velocity)
{
  if (m_dragging)
  {
    m_dragging = false;

    auto decelerationAnimation = decelerationAnimationWithVelocity(velocity);
    if (decelerationAnimation)
    {
      setScrollAnimation(decelerationAnimation);

      m_decelerating = true;
    }
    else
    {
      confineContent();
    }
  }
}

void UIScrollView::cancelScrollAnimation()
{
  VERBOSE("UIScrollView::cancelScrollAnimation");
  if (m_scrollTimer)
  {
    m_scrollTimer->invalidate();
    m_scrollTimer = nullptr;
  }

  m_scrollAnimation = nullptr;

  if (m_decelerating)
  {
    m_decelerating = false;
  }
}

Point UIScrollView::confinedContentOffset(Point contentOffset)
{
  const auto viewSize = size();

  if (contentOffset.x < viewSize.width - m_contentSize.width)
  {
    contentOffset.x = viewSize.width - m_contentSize.width;
  }
  if (contentOffset.y < viewSize.height - m_contentSize.height)
  {
    contentOffset.y = viewSize.height - m_contentSize.height;
  }

  contentOffset.x = std::min(contentOffset.x, 0.0);
  contentOffset.y = std::min(contentOffset.y, 0.0);

  return contentOffset;
}

void UIScrollView::setRestrainedContentOffset(Point offset)
{
  setContentOffset(offset);
}

std::shared_ptr<UIScrollViewAnimation> UIScrollView::decelerationAnimationWithVelocity(
  Point velocity)
{
  const auto confinedOffset = confinedContentOffset(m_contentOffset);

  if (confinedOffset.x != m_contentOffset.x)
  {
    velocity.x = 0;
  }
  if (confinedOffset.y != m_contentOffset.y)
  {
    velocity.y = 0;
  }

  if ((velocity != Point::zero()) || (confinedOffset != m_contentOffset))
  {
    return std::make_shared<UIScrollViewAnimationDeceleration>(this, velocity);
  }
  else
  {
    return nullptr;
  }
}

void UIScrollView::setScrollAnimation(std::shared_ptr<UIScrollViewAnimation> animation)
{
  cancelScrollAnimation();

  m_scrollAnimation = animation;

  if (!m_scrollTimer)
  {
    m_scrollTimer.reset(new Timer(
      1. / 60,
      [this]()
      {
        VERBOSE("UIScrollView: call updateScrollAnimation by timer");
        this->updateScrollAnimation();
      },
      true));
  }
}

void UIScrollView::updateScrollAnimation()
{
  if (m_scrollAnimation->animate())
  {
    DEBUG("UIScrollView::updateScrollAnimation: animation finished, cancel timer");
    cancelScrollAnimation();
  }
}

void UIScrollView::confineContent()
{
  setContentOffset(confinedContentOffset(m_contentOffset));
}
