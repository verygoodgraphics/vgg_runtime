#include "VggFloat.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace VGG
{
bool nearlyEqual(double x, double y)
{
  if (doubleNearlyZero(x))
  {
    return doubleNearlyZero(y);
  }
  return doublesNearlyEqual(x, y);
}

bool doubleNearlyZero(double a)
{
  return a == 0 || fabs(a) < std::numeric_limits<float>::epsilon();
}

bool doublesNearlyEqual(double a, double b)
{
  constexpr double TOLERANCE = 0.000001;
  return a == b || std::abs(b - a) < TOLERANCE;
}

} // namespace VGG
