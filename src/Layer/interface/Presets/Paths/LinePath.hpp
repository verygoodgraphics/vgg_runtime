/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __LINE_PATH_HPP__
#define __LINE_PATH_HPP__

#include "Components/Path.hpp"

namespace VGG
{

class LinePath
{
  using PM = CurvePoint::PointMode;

public:
  static inline Path create()
  {
    return Path{
      .isClosed = false,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 0. } },
        CurvePoint{ .point = Vec2{ 1., 1. } },
      },
    };
  }

  static inline Path createAnti()
  {
    return Path{
      .isClosed = false,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 1. } },
        CurvePoint{ .point = Vec2{ 1., 0. } },
      },
    };
  }
};

}; // namespace VGG

#endif // __LINE_PATH_HPP__
