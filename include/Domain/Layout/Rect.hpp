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

namespace VGG
{

namespace Layout
{

using Scalar = float;

constexpr auto FLIP_Y_FACTOR = -1;
#define TO_VGG_LAYOUT_SCALAR(x) static_cast<Layout::Scalar>(x)

struct Matrix
{
  Scalar a{ 1 };
  Scalar b{ 0 };
  Scalar c{ 0 };
  Scalar d{ 1 };
  Scalar tx{ 0 };
  Scalar ty{ 0 };

  bool operator==(const Matrix& rhs) const noexcept;
};

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

  Point makeFromModelPoint() const
  {
    return { x, y * FLIP_Y_FACTOR };
  }
  Point toModelPoint() const
  {
    return { x, y * FLIP_Y_FACTOR };
  }
  Point makeTransform(const Matrix& matrix) const
  {
    const auto [a, b, c, d, tx, ty] = matrix;
    return { a * x + c * y + tx, b * x + d * y + ty };
  }
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
    return { origin.toModelPoint(), size };
  }
  Rect makeOffset(Scalar dx, Scalar dy) const
  {
    return { { origin.x + dx, origin.y + dy }, size };
  }

  Rect makeIntersect(const Rect& rhs) const;
  Rect makeTransform(const Matrix& matrix, ECoordinateType type) const;

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
};

Matrix getAffineTransform(const std::array<Point, 3>& oldPoints,
                          const std::array<Point, 3>& newPoints);

} // namespace Layout

} // namespace VGG
