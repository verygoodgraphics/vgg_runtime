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

#include <array>
#include <vector>

namespace VGG
{

namespace Layout
{

struct BezierPoint;
struct Matrix;
struct Rect;

using Scalar = double;

constexpr auto FLIP_Y_FACTOR = -1;
#define TO_VGG_LAYOUT_SCALAR(x) static_cast<Layout::Scalar>(x)

struct Point
{
  Scalar x{ 0 };
  Scalar y{ 0 };

  bool operator==(const Point& rhs) const noexcept;
  bool operator!=(const Point& rhs) const noexcept
  {
    return !(*this == rhs);
  }

  Point operator+(const Point& rhs) const noexcept
  {
    return { x + rhs.x, y + rhs.y };
  }
  Point& operator+=(const Point& rhs) noexcept
  {
    return *this = *this + rhs;
  }
  Point operator-(const Point& rhs) const noexcept
  {
    return { x - rhs.x, y - rhs.y };
  }

  Point makeFromModelPoint() const
  {
    return { x, y * FLIP_Y_FACTOR };
  }
  Point makeModelPoint() const
  {
    return { x, y * FLIP_Y_FACTOR };
  }
  Point makeTransform(const Matrix& matrix) const;
  Point makeScale(const Rect& oldContainerFrame, const Rect& newContainerFrame) const;
};

struct Size
{
  Scalar width{ 0 };
  Scalar height{ 0 };

  bool operator==(const Size& rhs) const noexcept;
  bool operator!=(const Size& rhs) const noexcept
  {
    return !(*this == rhs);
  }
};

struct Rect
{
  enum class ECoordinateType
  {
    MODEL, // x: → y: ↑
    LAYOUT // x: → y: ↓
  };

  Point origin;
  Size size;

  bool contains(Point point)
  {
    return (point.x >= origin.x && point.x <= (origin.x + size.width)) &&
           (point.y >= origin.y && point.y <= (origin.y + size.height));
  }
  bool operator==(const Rect& rhs) const noexcept
  {
    return origin == rhs.origin && size == rhs.size;
  }
  bool operator!=(const Rect& rhs) const noexcept
  {
    return !(*this == rhs);
  }

  Rect makeFromModelRect() const
  {
    return { origin.makeFromModelPoint(), size };
  }
  Rect makeModelRect() const
  {
    return { origin.makeModelPoint(), size };
  }
  Rect makeOffset(Scalar dx, Scalar dy) const
  {
    return { { origin.x + dx, origin.y + dy }, size };
  }

  Rect makeJoin(const Rect& rhs) const;
  Rect& join(const Rect& rhs)
  {
    return *this = makeJoin(rhs);
  }

  Rect makeTransform(const Matrix& matrix, ECoordinateType type) const;
  static Rect makeFromPoints(const std::vector<Point>& points);
  static Rect makeFromPoints(const std::vector<BezierPoint>& points, bool isClosed);
  static Rect makeFromPoints(const BezierPoint p1, const BezierPoint p2);

  Scalar left() const
  {
    return origin.x;
  }

  Scalar top() const
  {
    return origin.y;
  }

  Scalar right() const
  {
    return origin.x + size.width;
  }

  Scalar bottom() const
  {
    return origin.y + size.height;
  }

  Scalar width() const
  {
    return size.width;
  }

  Scalar height() const
  {
    return size.height;
  }

  Scalar centerX() const
  {
    return left() + width() / 2;
  }

  Scalar centerY() const
  {
    return top() + height() / 2;
  }

  Point center() const
  {
    return { centerX(), centerY() };
  }

  void setCenter(const Point& newCenter)
  {
    origin.x = newCenter.x - width() / 2;
    origin.y = newCenter.y - height() / 2;
  }
};

} // namespace Layout

} // namespace VGG
