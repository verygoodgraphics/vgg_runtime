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
#include "Rect.hpp"

#include "Utility/VggFloat.hpp"

#include <algorithm>

using namespace VGG;
using namespace VGG::Layout;

bool Point::operator==(const Point& rhs) const noexcept
{
  return nearlyEqual(x, rhs.x) && nearlyEqual(y, rhs.y);
}

bool Size::operator==(const Size& rhs) const noexcept
{
  return nearlyEqual(width, rhs.width) && nearlyEqual(height, rhs.height);
}

bool Matrix::operator==(const Matrix& rhs) const noexcept
{
  return nearlyEqual(a, rhs.a) && nearlyEqual(b, rhs.b) && nearlyEqual(c, rhs.c) &&
         nearlyEqual(d, rhs.d) && nearlyEqual(tx, rhs.tx) && nearlyEqual(ty, rhs.ty);
}

Rect Rect::makeIntersect(const Rect& rhs) const
{
  auto minLeft = std::min(left(), rhs.left());
  auto minTop = std::min(top(), rhs.top());
  auto maxRight = std::max(right(), rhs.right());
  auto maxBottom = std::max(bottom(), rhs.bottom());
  return { { minLeft, minTop }, { maxRight - minLeft, maxBottom - minTop } };
}