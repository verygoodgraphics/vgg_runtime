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

#include <core/SkColor.h>

#include <glm/glm.hpp>

namespace VGG
{
struct Color
{
  float r{ 0. };
  float g{ 0. };
  float b{ 0. };
  float a{ 1. };

  operator SkColor() const
  {
    return SkColorSetARGB(255 * a, 255 * r, 255 * g, 255 * b);
  }

  inline static Color fromRGB(unsigned char ur, unsigned char ug, unsigned char ub)
  {
    return Color{ ur / 255.f, ug / 255.f, ub / 255.f, 1. };
  }

  inline static Color fromARGB(
    unsigned char ua,
    unsigned char ur,
    unsigned char ug,
    unsigned char ub)
  {
    return Color{ ur / 255.f, ug / 255.f, ub / 255.f, ua / 255.f };
  }
};

} // namespace VGG
