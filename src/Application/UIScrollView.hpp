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

#include "Application/UIView.hpp"

#include "UIPanGestureRecognizer.hpp"
#include "Types.hpp"

#include <Utility/VggTimer.hpp>

#include <memory>

using VGG::Layout::Point;
using VGG::Layout::Size;
namespace VGG
{

namespace UIKit
{
class UIScrollViewAnimation;
class UIScrollViewAnimationDeceleration;
} // namespace UIKit

class UIScrollView : public UIView
{
  friend class UIKit::UIScrollViewAnimationDeceleration;

public:
private:
  Size  m_contentSize{ 0, 0 };
  Point m_contentOffset{ 0, 0 };

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

  void setContentSize(Size size);
  void setContentOffset(Point offset, bool byMouseWheel = false);
  auto contentOffset() const -> Point
  {
    return m_contentOffset;
  }

private:
  bool canScrollHorizontal();
  bool canScrollVertical();

  void handleGesture(UIKit::UIPanGestureRecognizer& recognizer);

  void beginDragging();
  void dragBy(Point delta);
  void endDraggingWithDecelerationVelocity(Point velocity);

  void setScrollAnimation(std::shared_ptr<UIKit::UIScrollViewAnimation> animation);
  void cancelScrollAnimation();
  void updateScrollAnimation();

  std::shared_ptr<UIKit::UIScrollViewAnimation> decelerationAnimationWithVelocity(Point velocity);

  void  confineContent();
  void  setRestrainedContentOffset(Point offset);
  Point confinedContentOffset(Point contentOffset);
};

} // namespace VGG
