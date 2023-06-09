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
#ifndef __FRAME_HPP__
#define __FRAME_HPP__

#include <nlohmann/json.hpp>
#include "include/core/SkRect.h"
#include <tuple>

#include "Utils/Math.hpp"
#include "Utils/Utils.hpp"

namespace VGG
{

struct Frame
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Frame, x, y, w, h, rotation, flipX, flipY, globalOffset);

  static constexpr double minRotation = -180.0;
  static constexpr double maxRotation = 180.0;
  double x{ 0. }; // relative to container's frame if exists
  double y{ 0. }; // relative to container's frame if exists
  double w{ 0. };
  double h{ 0. };
  Vec2 globalOffset{ 0., 0. };
  double rotation{ 0. }; // degree
  bool flipX{ false };
  bool flipY{ false };

  inline bool isEmpty()
  {
    return (std::fabs(w) < FTOLER) || (std::fabs(h) < FTOLER);
  }

  inline auto posFromNormalized(double nx, double ny) const
  {
    return Vec2{ w * nx, h * ny };
  }

  inline auto globalPos() const
  {
    return Vec2{ x + globalOffset.x, y + globalOffset.y };
  }

  inline auto globalPosFromNormalized(double nx, double ny) const
  {
    auto [x, y] = globalPos();
    return Vec2{ x + w * nx, y + h * ny };
  }

  inline auto globalAligns() const
  {
    auto [x, y] = globalPos();
    return std::make_tuple(x, x + w / 2, x + w, y, y + h / 2, y + h);
  }

  inline auto normalizedPosFromGlobal(double gx, double gy) const
  {
    ASSERT(w > 0 && h > 0);
    auto [x, y] = globalPos();
    return Vec2{ (gx - x) / w, (gy - y) / h };
  }

  inline double normalizedSizeFromGlobal(double gs) const
  {
    ASSERT(w > 0 && h > 0);
    return gs / std::max(w, h);
  }

  inline auto mousePosFromGlobal(double mx, double my) const
  {
    auto [gx, gy] = globalPos();
    auto ox = gx + w / 2;
    auto oy = gy + h / 2;
    auto t = -deg2rad(rotation);
    return Vec2{
      (mx - ox) * std::cos(t) - (my - oy) * std::sin(t) + ox,
      (mx - ox) * std::sin(t) + (my - oy) * std::cos(t) + oy,
    };
  }

  inline void join(const Frame& other)
  {
    if (w <= 0.0 || h <= 0.0)
    {
      *this = other;
      return;
    }
    auto [x1, y1] = globalPos();
    auto [x2, y2] = other.globalPos();
    double xmin = std::min(x1, x2);
    double xmax = std::max(x1 + w, x2 + other.w);
    double ymin = std::min(y1, y2);
    double ymax = std::max(y1 + h, y2 + other.h);
    x = xmin - globalOffset.x;
    y = ymin - globalOffset.y;
    w = std::max(0.0, xmax - xmin);
    h = std::max(0.0, ymax - ymin);
  }

  inline SkRect toLocalSkRect() const
  {
    SkRect rect;
    rect.setXYWH(0, 0, w, h);
    return rect;
  }

  inline SkRect toSkRect() const
  {
    SkRect rect;
    rect.setXYWH(x, y, w, h);
    return rect;
  }

  inline SkRect toGlobalSkRect() const
  {
    SkRect rect;
    auto [x, y] = globalPos();
    rect.setXYWH(x, y, w, h);
    return rect;
  }
};

}; // namespace VGG

#endif // __FRAME_HPP__
