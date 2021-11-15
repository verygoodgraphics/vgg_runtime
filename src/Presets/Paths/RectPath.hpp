/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
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
#ifndef __RECT_PATH_HPP__
#define __RECT_PATH_HPP__

#include "Components/Path.hpp"

namespace VGG
{

class RectPath
{
  using PM = CurvePoint::PointMode;

public:
  static inline Path create()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0., 0. } },
        CurvePoint{ .point = Vec2{ 0., 1. } },
        CurvePoint{ .point = Vec2{ 1., 1. } },
        CurvePoint{ .point = Vec2{ 1., 0. } },
      },
    };
  }

  static inline Path createRounded()
  {
    Path p = RectPath::create();
    for (size_t i = 0; i < p.points.size(); i++)
    {
      p.points[i].radius = 5.;
    }
    return p;
  }

  static inline Path createPolygon(double radius, int nEdges, bool isUpward)
  {
    ASSERT(radius >= 0);
    ASSERT(nEdges > 2);
    Vec2 center{ 0.5, 0.5 };
    Vec2 sp = isUpward ? Vec2{ 0.5, 0.0 } : Vec2{ 0.0, 0.5 };
    if (nEdges == 4 && !isUpward)
    {
      // TODO for nEdges % 4 == 0
      sp = Vec2{ 0.0, 0.0 };
    }
    Vec2 v = sp - center;

    std::vector<CurvePoint> pts;
    pts.push_back(CurvePoint{ .radius = radius, .point = sp });

    double dt = 2 * M_PI / nEdges;
    for (size_t i = 1; i < nEdges; i++)
    {
      double theta = -dt * i;
      Vec2 vv = Vec2{ std::cos(theta) * v.x + std::sin(theta) * v.y,
                      std::sin(-theta) * v.x + std::cos(theta) * v.y };
      Vec2 p = center + vv;
      pts.push_back(CurvePoint{ .radius = radius, .point = p });
    }
    return Path{
      .isClosed = true,
      .points = pts,
    };
  }
};

}; // namespace VGG

#endif // __RECT_PATH_HPP__
