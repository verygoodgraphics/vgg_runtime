#include "Rect.hpp"

#include "VggFloat.hpp"

using namespace VGG;
using namespace VGG::Layout;

bool Point::operator==(const Point& rhs) const noexcept
{
  return nearly_equal(x, rhs.x) && nearly_equal(y, rhs.y);
}

bool Size::operator==(const Size& rhs) const noexcept
{
  return nearly_equal(width, rhs.width) && nearly_equal(height, rhs.height);
}

bool Matrix::operator==(const Matrix& rhs) const noexcept
{
  return nearly_equal(a, rhs.a) && nearly_equal(b, rhs.b) && nearly_equal(c, rhs.c) &&
         nearly_equal(d, rhs.d) && nearly_equal(tx, rhs.tx) && nearly_equal(ty, rhs.ty);
}