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

#include "Layer/Core/VBounds.hpp"

#include <glm/glm.hpp>

#ifdef STD_FORMAT_SUPPORT
template<>
struct std::formatter<VGG::Bounds>
{
  constexpr auto parse(std::format_parse_context& ctx)
  {
    return ctx.begin();
  }

  auto format(const VGG::Bounds& s, std::format_context& ctx) const
  {
    return std::format_to(
      ctx.out(),
      "[{}, {}, {}, {}]",
      s.topLeft().x,
      s.topLeft().y,
      s.width(),
      s.height());
  }
};

template<>
struct std::formatter<glm::mat3>
{
  constexpr auto parse(std::format_parse_context& ctx)
  {
    return ctx.begin();
  }

  auto format(const glm::mat3& s, std::format_context& ctx) const
  {
    return std::format_to(
      ctx.out(),
      "[{}, {}, {},\n {}, {}, {}, \n{}, {}, {}]",
      s[0][0],
      s[0][1],
      s[0][2],
      s[1][0],
      s[1][1],
      s[1][2],
      s[2][0],
      s[2][1],
      s[2][2]);
  }
};

#endif
