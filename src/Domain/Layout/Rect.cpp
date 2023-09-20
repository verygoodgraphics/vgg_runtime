#include "Rect.hpp"

#include "Utility/VggFloat.hpp"

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