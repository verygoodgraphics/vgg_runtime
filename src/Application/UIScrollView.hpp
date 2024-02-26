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
#pragma once

#include "UIView.hpp"

#include "Domain/Layout/Rect.hpp"

namespace VGG
{

class UIScrollView : public UIView
{
public:
  using UISize = Layout::Size;
  using UIPoint = Layout::Point;

private:
  UISize  m_contentSize{ 0, 0 };
  UIPoint m_contentOffset{ 0, 0 };

public:
  bool onEvent(UEvent e, void* userData) override;

  void setContentSize(UISize size);
  void setContentOffset(UIPoint offset);

  void endDraggingWithDecelerationVelocity(UIPoint velocity);

private:
  bool canScrollHorizontal();
  bool canScrollVertical();
};

} // namespace VGG
