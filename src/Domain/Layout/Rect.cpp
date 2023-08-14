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
