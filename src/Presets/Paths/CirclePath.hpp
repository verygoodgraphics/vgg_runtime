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
#ifndef __CIRCLE_PATH_HPP__
#define __CIRCLE_PATH_HPP__

#include "Components/Path.hpp"

namespace VGG
{

class CirclePath
{
  using PM = CurvePoint::PointMode;

public:
  static inline Path create()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{
          .mode = PM::MIRRORED,
          .point = Vec2{ .5, 1. },
          .from = Vec2{ 0.7761423749, 1. },
          .to = Vec2{ 0.2238576251, 1 },
        },
        CurvePoint{
          .mode = PM::MIRRORED,
          .point = Vec2{ 1., .5 },
          .from = Vec2{ 1., 0.2238576251 },
          .to = Vec2{ 1., 0.7761423749 },
        },
        CurvePoint{
          .mode = PM::MIRRORED,
          .point = Vec2{ .5, 0. },
          .from = Vec2{ 0.2238576251, 0. },
          .to = Vec2{ 0.7761423749, 0. },
        },
        CurvePoint{
          .mode = PM::MIRRORED,
          .point = Vec2{ 0., .5 },
          .from = Vec2{ 0., 0.7761423749 },
          .to = Vec2{ 0., 0.2238576251 },
        },
      },
    };
  }
};

}; // namespace VGG

#endif // __CIRCLE_PATH_HPP__
