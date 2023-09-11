#pragma once

namespace VGG
{

namespace Layout
{

using Scalar = float;

#define TO_VGG_LAYOUT_SCALAR(x) static_cast<Layout::Scalar>(x)

struct Point
{
  Scalar x{ 0 };
  Scalar y{ 0 };

  bool operator==(const Point& rhs) const noexcept;
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
};

struct Matrix
{
  Scalar a;
  Scalar b;
  Scalar c;
  Scalar d;
  Scalar tx;
  Scalar ty;

  bool operator==(const Matrix& rhs) const noexcept;
};

} // namespace Layout

} // namespace VGG