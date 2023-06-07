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
#ifndef __STAR_PATH_HPP__
#define __STAR_PATH_HPP__

#include "Components/Path.hpp"

namespace VGG
{

class StarPath
{
  using PM = CurvePoint::PointMode;

public:
  static inline Path create()
  {
    return Path{
      .isClosed = true,
      .points = {
        CurvePoint{ .point = Vec2{ 0.5, 0.75 } },
        CurvePoint{ .point = Vec2{ 0.20610737385376349, 0.90450849718747373 } },
        CurvePoint{ .point = Vec2{ 0.26223587092621159, 0.57725424859373686 } },
        CurvePoint{ .point = Vec2{ 0.024471741852423179, 0.34549150281252639 } },
        CurvePoint{ .point = Vec2{ 0.35305368692688166, 0.29774575140626314 } },
        CurvePoint{ .point = Vec2{ 0.5, 0. } },
        CurvePoint{ .point = Vec2{ 0.64694631307311823, 0.29774575140626314 } },
        CurvePoint{ .point = Vec2{ 0.97552825814757682, 0.34549150281252616 } },
        CurvePoint{ .point = Vec2{ 0.73776412907378841, 0.57725424859373675 } },
        CurvePoint{ .point = Vec2{ 0.79389262614623668, 0.90450849718747361 } },
      },
    };
  }

  static inline Path createStar(double radius, int nPoints)
  {
    ASSERT(radius >= 0);
    ASSERT(nPoints > 2);
    Vec2 center{ 0.5, 0.5 };
    Vec2 sp{ 0.5, 0.0 };
    Vec2 v = sp - center;

    std::vector<CurvePoint> pts;
    double dt = M_PI / nPoints;
    for (size_t i = 0; i < nPoints * 2; i++)
    {
      double theta = dt * i;
      Vec2 vv = Vec2{ std::cos(theta) * v.x + std::sin(theta) * v.y,
                      std::sin(-theta) * v.x + std::cos(theta) * v.y };
      Vec2 p = center + vv.scale(i % 2 == 0 ? 1.0 : radius);
      pts.push_back(CurvePoint{ .point = p });
    }
    return Path{
      .isClosed = true,
      .points = pts,
    };
  }
};

}; // namespace VGG

#endif // __STAR_PATH_HPP__
