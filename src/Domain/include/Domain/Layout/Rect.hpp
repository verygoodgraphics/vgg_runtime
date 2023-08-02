#pragma once

namespace VGG
{

namespace Layout
{

using Scalar = double;

#define LayoutIntToScalar(x) static_cast<Scalar>(x)

struct Point
{
  Scalar x{ 0 };
  Scalar y{ 0 };
};

struct Size
{
  Scalar width{ 0 };
  Scalar height{ 0 };
};

struct Rect
{
  Point origin;
  Size size;

  bool pointInRect(Point point)
  {
    return (point.x >= origin.x && point.x <= (origin.x + size.width)) &&
           (point.y >= origin.y && point.y <= (origin.y + size.height));
  }
};

} // namespace Layout

} // namespace VGG