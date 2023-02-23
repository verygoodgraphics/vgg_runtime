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
#ifndef __PATH_HPP__
#define __PATH_HPP__

#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#include "Components/Frame.hpp"
#include "Components/Styles.hpp"
#include "Utils/Math.hpp"
#include "Utils/Types.hpp"

namespace VGG
{

struct CurvePoint
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(CurvePoint, mode, radius, point, from, to);

  static constexpr double SIZE = 3.0;
  static constexpr double minRadius = 0.0;

  enum class PointMode
  {
    STRAIGHT = 1,
    MIRRORED = 2,
    ASYMMETRIC = 3,
    DISCONNECTED = 4,
  };

  PointMode mode{ PointMode::STRAIGHT };
  double radius{ 0. };

  // points are all in normalized space [0,1]x[0,1]
  Vec2 point{ 0., 0. };
  std::optional<Vec2> from{ std::nullopt };
  std::optional<Vec2> to{ std::nullopt };

  inline void assureControlPoints()
  {
    if (!from)
    {
      from = point;
    }
    if (!to)
    {
      to = point;
    }
  }
};

struct Path
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Path, isClosed, points, special);

  enum class Specialization
  {
    NORMAL = 0,
    RECTANGLE, // "fixedRadius"
    OVAL,
    TRIANGLE,
    STAR,    // "numberOfPoints" "radius"
    POLYGON, // "numberOfPoints"
  };

  bool isClosed{ false };
  std::vector<CurvePoint> points;
  Specialization special{ Specialization::NORMAL };

  inline bool isStraightLine() const
  {
    using PM = CurvePoint::PointMode;
    if (points.size() == 2)
    {
      if (points[0].mode == PM::STRAIGHT && points[1].mode == PM::STRAIGHT)
      {
        return true;
      }
      else if (!(points[0].from.has_value() || points[1].to.has_value()))
      {
        if (isClosed && (points[1].from.has_value() || points[0].to.has_value()))
        {
          return false;
        }
        return true;
      }
    }
    return false;
  }
};

struct FramedPath
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(FramedPath, frame, path, style);

  Frame frame;
  Path path;
  PathStyle style;

  inline bool isFilled() const
  {
    if (path.isStraightLine())
    {
      return false;
    }
    return style.isFilled();
  }
};

}; // namespace VGG

#endif // __PATH_HPP__
