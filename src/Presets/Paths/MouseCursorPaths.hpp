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
#ifndef __MOUSE_CURSOR_PATHS_HPP__
#define __MOUSE_CURSOR_PATHS_HPP__

#include "Components/Path.hpp"

namespace VGG
{

class MouseCursorPaths
{
  using PM = CurvePoint::PointMode;

public:
  static inline Path createPointer()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 0. } },
        CurvePoint{ .point = Vec2{ 0., 1. } },
        CurvePoint{ .point = Vec2{ 0.28, 0.7 } },
        CurvePoint{ .point = Vec2{ 0.7, 0.7 } },
      },
    };
  }

  static inline Path createHorzBiArrow()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 0.5 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.3 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.3 } },
        CurvePoint{ .point = Vec2{ 1., 0.5 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.7 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.7 } },
      },
    };
  }

  static inline Path createVertBiArrow()
  {
    Path p = createHorzBiArrow();
    for (size_t i = 0; i < p.points.size(); i++)
    {
      std::swap(p.points[i].point.x, p.points[i].point.y);
    }
    return p;
  }

  static inline Path createDiagBiArrow()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0.15, 0.15 } },
        CurvePoint{ .point = Vec2{ 0.15, 0.43 } },
        CurvePoint{ .point = Vec2{ 0.26, 0.32 } },
        CurvePoint{ .point = Vec2{ 0.68, 0.74 } },
        CurvePoint{ .point = Vec2{ 0.57, 0.85 } },
        CurvePoint{ .point = Vec2{ 0.85, 0.85 } },
        CurvePoint{ .point = Vec2{ 0.85, 0.57 } },
        CurvePoint{ .point = Vec2{ 0.74, 0.68 } },
        CurvePoint{ .point = Vec2{ 0.32, 0.26 } },
        CurvePoint{ .point = Vec2{ 0.43, 0.15 } },
      },
    };
  }

  static inline Path createAntiDiagBiArrow()
  {
    Path p = createDiagBiArrow();
    for (size_t i = 0; i < p.points.size(); i++)
    {
      p.points[i].point.x = 1 - p.points[i].point.x;
    }
    return p;
  }

  static inline Path createCrossBiArrows()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 0.5 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.3 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.45, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.45, 0.2 } },
        CurvePoint{ .point = Vec2{ 0.3, 0.2 } },
        CurvePoint{ .point = Vec2{ 0.5, 0. } },
        CurvePoint{ .point = Vec2{ 0.7, 0.2 } },
        CurvePoint{ .point = Vec2{ 0.55, 0.2 } },
        CurvePoint{ .point = Vec2{ 0.55, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.3 } },
        CurvePoint{ .point = Vec2{ 1., 0.5 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.7 } },
        CurvePoint{ .point = Vec2{ 0.8, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.55, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.55, 0.8 } },
        CurvePoint{ .point = Vec2{ 0.7, 0.8 } },
        CurvePoint{ .point = Vec2{ 0.5, 1. } },
        CurvePoint{ .point = Vec2{ 0.3, 0.8 } },
        CurvePoint{ .point = Vec2{ 0.45, 0.8 } },
        CurvePoint{ .point = Vec2{ 0.45, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.2, 0.7 } },
      },
    };
  }

  static inline Path createCrossLines()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 0.45 } },
        CurvePoint{ .point = Vec2{ 0., 0.55 } },
        CurvePoint{ .point = Vec2{ 0.45, 0.55 } },
        CurvePoint{ .point = Vec2{ 0.45, 1. } },
        CurvePoint{ .point = Vec2{ 0.55, 1. } },
        CurvePoint{ .point = Vec2{ 0.55, 0.55 } },
        CurvePoint{ .point = Vec2{ 1., 0.55 } },
        CurvePoint{ .point = Vec2{ 1., 0.45 } },
        CurvePoint{ .point = Vec2{ 0.55, 0.45 } },
        CurvePoint{ .point = Vec2{ 0.55, 0. } },
        CurvePoint{ .point = Vec2{ 0.45, 0. } },
        CurvePoint{ .point = Vec2{ 0.45, 0.45 } },
      },
    };
  }

  static inline Path createTextInput()
  {
    return Path{
      .isClosed = false,
      .points = {
        CurvePoint{ .point = Vec2{ 0.4, 0. } },
        CurvePoint{ .point = Vec2{ 0.5, 0.05 } },
        CurvePoint{ .point = Vec2{ 0.6, 0. } },
        CurvePoint{ .point = Vec2{ 0.5, 0.05 } },
        CurvePoint{ .point = Vec2{ 0.5, 0.95 } },
        CurvePoint{ .point = Vec2{ 0.4, 1. } },
        CurvePoint{ .point = Vec2{ 0.5, 0.95 } },
        CurvePoint{ .point = Vec2{ 0.6, 1. } },
      },
    };
  }
};

}; // namespace VGG

#endif // __MOUSE_CURSOR_PATHS_HPP__
