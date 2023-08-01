#pragma once

namespace VGG
{

namespace Layout
{

using Scalar = double;

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
};

} // namespace Layout

} // namespace VGG