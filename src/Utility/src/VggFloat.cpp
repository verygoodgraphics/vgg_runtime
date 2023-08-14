#include "VggFloat.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace VGG
{
bool nearly_equal(double x, double y)
{
  if (double_nearly_zero(x))
  {
    return double_nearly_zero(y);
  }
  return doubles_nearly_equal(x, y);
}

bool double_nearly_zero(double a)
{
  return a == 0 || fabs(a) < std::numeric_limits<float>::epsilon();
}

bool doubles_nearly_equal(double a, double b)
{
  constexpr double tolerance = 0.000001;
  return a == b || std::abs(b - a) < tolerance;
}

} // namespace VGG
