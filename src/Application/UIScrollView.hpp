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
#include "Application/UIView.hpp"
#include "Domain/Layout/Rect.hpp"
#include "UIPanGestureRecognizer.hpp"
namespace VGG
{
class Timer;
namespace UIKit
{
class UIScrollViewAnimation;
class UIScrollViewAnimationDeceleration;
} // namespace UIKit
} // namespace VGG
union UEvent;

namespace VGG
{

class UIScrollView : public UIView
{
  friend class UIKit::UIScrollViewAnimationDeceleration;

private:
  Layout::Size  m_contentSize{ 0, 0 };
  Layout::Point m_contentOffset{ 0, 0 };

  bool m_scrollEnabled{ true };
  bool m_bouncesHorizontally{ false };
  bool m_bouncesVertically{ true };
  bool m_dragging{ false };
  bool m_decelerating{ false };

  std::shared_ptr<Timer> m_scrollTimer;

  UIKit::UIPanGestureRecognizer m_panGestureRecognizer;

  std::shared_ptr<UIKit::UIScrollViewAnimation> m_scrollAnimation;

public:
  bool isScrollEnabled() const
  {
    return m_scrollEnabled;
  }
  void setScrollEnabled(bool enabled)
  {
    m_scrollEnabled = enabled;
  }

  bool boucesHorizontally() const
  {
    return m_bouncesHorizontally;
  }
  bool boucesVertically() const
  {
    return m_bouncesVertically;
  }

public:
  UIScrollView();

  bool onEvent(UEvent e, void* userData) override;

  Layout::Size contentSize() const;
  void         setContentSize(Layout::Size size);
  void         setContentOffset(Layout::Point offset, bool cancelAnimation = false);
  auto         contentOffset() const -> Layout::Point
  {
    return m_contentOffset;
  }

private:
  bool canScrollHorizontal();
  bool canScrollVertical();

  void handleGesture(UIKit::UIPanGestureRecognizer& recognizer);

  void beginDragging();
  void dragBy(Layout::Point delta);
  void endDraggingWithDecelerationVelocity(Layout::Point velocity);

  void setScrollAnimation(std::shared_ptr<UIKit::UIScrollViewAnimation> animation);
  void cancelScrollAnimation();
  void updateScrollAnimation();

  std::shared_ptr<UIKit::UIScrollViewAnimation> decelerationAnimationWithVelocity(
    Layout::Point velocity);

  void          confineContent();
  void          setRestrainedContentOffset(Layout::Point offset);
  Layout::Point confinedContentOffset(Layout::Point contentOffset);
};

} // namespace VGG
