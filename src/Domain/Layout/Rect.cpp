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
#include "Rect.hpp"
#include <algorithm>
#include <optional>
#include <ostream>
#include "BezierPoint.hpp"
#include "Math.hpp"
#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"
#include <bezier.h>

namespace VGG::Layout
{

bool Point::operator==(const Point& rhs) const noexcept
{
  return nearlyEqual(x, rhs.x) && nearlyEqual(y, rhs.y);
}
Point Point::makeTransform(const Matrix& matrix) const
{
  const auto [a, b, c, d, tx, ty] = matrix;
  return { a * x + c * y + tx, b * x + d * y + ty };
}

Point Point::makeScale(const Rect& oldContainerFrame, const Rect& newContainerFrame) const
{
  const auto newOrigin = newContainerFrame.origin;
  const auto [newW, newH] = newContainerFrame.size;
  const auto xRatio =
    oldContainerFrame.width() == 0 ? 0 : (x - oldContainerFrame.left()) / oldContainerFrame.width();
  const auto yRatio = oldContainerFrame.height() == 0
                        ? 0
                        : (y - oldContainerFrame.top()) / oldContainerFrame.height();
  return { newOrigin.x + xRatio * newW, newOrigin.y + yRatio * newH };
}

Point& Point::scale(const Scalar xScale, const Scalar yScale)
{
  x *= xScale;
  y *= yScale;
  return *this;
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

Rect Rect::makeJoin(const Rect& rhs) const
{
  auto minLeft = std::min(left(), rhs.left());
  auto minTop = std::min(top(), rhs.top());
  auto maxRight = std::max(right(), rhs.right());
  auto maxBottom = std::max(bottom(), rhs.bottom());
  return { { minLeft, minTop }, { maxRight - minLeft, maxBottom - minTop } };
}

Rect Rect::makeIntersectOrJoin(const Rect& rhs) const
{
  auto newLeft = std::max(left(), rhs.left());
  auto newTop = std::max(top(), rhs.top());
  auto newRight = std::min(right(), rhs.right());
  auto newBottom = std::min(bottom(), rhs.bottom());

  if (newLeft >= newRight || newTop >= newBottom)
  {
    return makeJoin(rhs);
  }

  return { { newLeft, newTop }, { newRight - newLeft, newBottom - newTop } };
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

Rect Rect::makeFromPoints(const std::vector<Point>& points)
{
  ASSERT(points.size() > 1);
  if (points.empty())
  {
    return {};
  }

  auto left = points[0].x;
  auto right = points[0].x;
  auto top = points[0].y;
  auto bottom = points[0].y;

  for (auto& point : points)
  {
    top = std::min(top, point.y);
    bottom = std::max(bottom, point.y);
    left = std::min(left, point.x);
    right = std::max(right, point.x);
  }

  return { { left, top }, { right - left, bottom - top } };
}

Rect Rect::makeFromPoints(const std::vector<BezierPoint>& points, bool isClosed)
{
  if (points.empty())
  {
    WARN("Rect::makeFromPoints: empty points");
    return {};
  }

  Rect result{ points[0].point, {} };

  if (points.size() == 1)
  {
    return result;
  }

  for (auto it = points.cbegin(); it + 1 != points.cend(); ++it)
  {
    auto partRect = makeFromPoints(*it, *(it + 1));
    result.join(partRect);
  }

  // last point
  if (isClosed)
  {
    auto partRect = makeFromPoints(points.back(), points.front());
    result.join(partRect);
  }

  return result;
}

Rect Rect::makeFromPoints(const BezierPoint p1, const BezierPoint p2)
{
  std::vector<bezier::Point> libPoints;
  libPoints.emplace_back(p1.point.x, p1.point.y);
  if (p1.from.has_value())
  {
    libPoints.emplace_back(p1.from->x, p1.from->y);
  }
  if (p2.to.has_value())
  {
    libPoints.emplace_back(p2.to->x, p2.to->y);
  }
  libPoints.emplace_back(p2.point.x, p2.point.y);
  switch (libPoints.size())
  {
    case 2:
    {
      return makeFromPoints({ p1.point, p2.point });
    }
    break;

    case 3:
    {
      bezier::Bezier<2> curve{ libPoints };
      auto              r = curve.aabb();
      return { { TO_VGG_LAYOUT_SCALAR(r.minX()), TO_VGG_LAYOUT_SCALAR(r.minY()) },
               { TO_VGG_LAYOUT_SCALAR(r.width()), TO_VGG_LAYOUT_SCALAR(r.height()) } };
    }
    break;

    case 4:
    {
      bezier::Bezier<3> curve{ libPoints };
      auto              r = curve.aabb();
      return { { TO_VGG_LAYOUT_SCALAR(r.minX()), TO_VGG_LAYOUT_SCALAR(r.minY()) },
               { TO_VGG_LAYOUT_SCALAR(r.width()), TO_VGG_LAYOUT_SCALAR(r.height()) } };
    }
    break;

    default:
      break;
  }

  return {};
}

std::ostream& operator<<(std::ostream& os, const Point& p)
{
  return os << "[" << p.x << ", " << p.y << "]";
}

std::ostream& operator<<(std::ostream& os, const Size& s)
{
  return os << "[" << s.width << ", " << s.height << "]";
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
  return os << "[" << r.origin << ", " << r.size << "]";
}

} // namespace VGG::Layout