/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include <cstdint>
#include <cmath>

#ifdef _MSC_VER
#include <limits>
#endif

namespace VGG
{
bool nearlyEqual(double x, double y);

bool doubleNearlyZero(double a);
bool doublesNearlyEqual(double a, double b);

inline bool floatNearlyZero(float a)
{
  return a == 0 || std::fabs(a) < std::numeric_limits<float>::epsilon();
}
inline bool floatsNearlyEqual(float a, float b)
{
  constexpr float TOLERANCE = 0.001;
  return a == b || std::abs(b - a) < TOLERANCE;
}
inline bool floatNearlyEqual(float a, float b)
{
  if (floatNearlyZero(a))
  {
    return floatNearlyZero(b);
  }
  return floatsNearlyEqual(a, b);
}

} // namespace VGG
