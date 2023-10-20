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

Rect Rect::makeTransform(const Matrix& matrix, ECoordinateType type) const
{
  const auto isYAxisDown = type == ECoordinateType::LAYOUT;

  auto x1 = left();
  auto y1 = top();
  auto x2 = right();
  auto y2 = isYAxisDown ? bottom() : y1 - height();

  Point p1{ x1, y1 };
  Point p2{ x2, y1 };
  Point p3{ x1, y2 };
  Point p4{ x2, y2 };

  p1 = p1.makeTransform(matrix);
  p2 = p2.makeTransform(matrix);
  p3 = p3.makeTransform(matrix);
  p4 = p4.makeTransform(matrix);

  auto minX = std::min(std::min(p1.x, p2.x), std::min(p3.x, p4.x));
  auto maxX = std::max(std::max(p1.x, p2.x), std::max(p3.x, p4.x));
  auto minY = std::min(std::min(p1.y, p2.y), std::min(p3.y, p4.y));
  auto maxY = std::max(std::max(p1.y, p2.y), std::max(p3.y, p4.y));

  return { { minX, isYAxisDown ? minY : maxY }, { maxX - minX, maxY - minY } };
}