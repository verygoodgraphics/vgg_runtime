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

#include "UIScrollViewAnimation.hpp"

namespace VGG::UIKit
{

struct UIScrollViewAnimationDecelerationComponent
{
  double decelerateTime{ 0 };
  double position{ 0 };
  double velocity{ 0 };
  double returnTime{ 0 };
  double returnFrom{ 0 };
  bool   bounced{ false };
};

class UIScrollViewAnimationDeceleration : public UIScrollViewAnimation
{
  UIScrollViewAnimationDecelerationComponent m_x;
  UIScrollViewAnimationDecelerationComponent m_y;
  double                                     m_lastMomentumTime;

public:
  UIScrollViewAnimationDeceleration(UIScrollView* sv, Point v);

  virtual bool animate() override;
  virtual void momentumScrollBy(Point delta) override;
};

} // namespace VGG::UIKit