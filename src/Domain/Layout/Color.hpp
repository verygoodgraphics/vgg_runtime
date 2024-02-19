/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include <nlohmann/json.hpp>
namespace VGG
{
namespace Layout
{
struct Color
{
  float alpha;
  float red;
  float green;
  float blue;

  uint32_t u32()
  {
    uint8_t a = 255 * alpha;
    uint8_t r = 255 * red;
    uint8_t g = 255 * green;
    uint8_t b = 255 * blue;

    return (a << 24) | (r << 16) | (g << 8) | (b << 0);
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Color, alpha, red, green, blue);

} // namespace Layout
} // namespace VGG