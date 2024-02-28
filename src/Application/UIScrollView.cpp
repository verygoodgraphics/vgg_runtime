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

#include <algorithm>
#include <cmath>

using namespace VGG;
using namespace VGG::UIKit;

UIScrollView::UIScrollView()
  : UIView()
{
  m_panGestureRecognizer.setHandler([this](UIPanGestureRecognizer& pan)
                                    { this->handleGesture(pan); });
}

bool UIScrollView::onEvent(UEvent event, void* userData)
{
  if (isScrollEnabled() && (canScrollHorizontal() || canScrollVertical()))
  {
    switch (event.type)
    {
      case VGG_MOUSEBUTTONDOWN:
        m_panGestureRecognizer.touchesBegan(event);
        break;

      case VGG_MOUSEMOTION:
        m_panGestureRecognizer.touchesMoved(event);
        break;

      case VGG_MOUSEBUTTONUP:
        m_panGestureRecognizer.touchesEnded(event);
        break;

      case VGG_TOUCHDOWN:
        break;
      case VGG_TOUCHMOTION:
        break;
      case VGG_TOUCHUP:
        break;

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

void UIScrollView::setContentOffset(Point offset)
{
  m_contentOffset = offset;
  setOffset(Offset{ offset.x, offset.y });
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
      beginDragging();
      break;

    case EUIGestureRecognizerState::CHANGED:
      dragBy(m_panGestureRecognizer.translation());
      m_panGestureRecognizer.setTranslation(Point::zero());
      break;

    case EUIGestureRecognizerState::ENDED:
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

    if (m_bounces)
    {
      bool shouldHorizontalBounce = (std::abs(proposedOffset.x - confinedOffset.x) > 0);
      bool shouldVerticalBounce = (std::abs(proposedOffset.y - confinedOffset.y) > 0);

      if (shouldHorizontalBounce)
      {
        proposedOffset.x = originalOffset.x + (0.055 * delta.x);
      }

      if (shouldVerticalBounce)
      {
        proposedOffset.y = originalOffset.y + (0.055 * delta.y);
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
  m_scrollTimer = false;

  m_scrollAnimation = nullptr;

  if (m_decelerating)
  {
    m_decelerating = false;
  }
}

Point UIScrollView::confinedContentOffset(Point contentOffset)
{
  contentOffset.x = std::max(contentOffset.x, 0.0);
  contentOffset.y = std::max(contentOffset.y, 0.0);

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
    m_scrollTimer = true; // call updateScrollAnimation
  }
}

void UIScrollView::updateScrollAnimation()
{
  if (m_scrollAnimation->animate())
  {
    cancelScrollAnimation();
  }
}

void UIScrollView::confineContent()
{
  setContentOffset(confinedContentOffset(m_contentOffset));
}