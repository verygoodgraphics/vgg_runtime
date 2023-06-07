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
#ifndef __GEOMETRY_SYSTEM_HPP__
#define __GEOMETRY_SYSTEM_HPP__

#include <skia/include/core/SkPath.h>

#include "Components/Path.hpp"
#include "Utils/Math.hpp"

namespace VGG
{

namespace GeometrySystem
{

using PM = CurvePoint::PointMode;

double calcRadius(double r0,
                  const Vec2& p0,
                  const Vec2& p1,
                  const Vec2& p2,
                  Vec2* left = nullptr,
                  Vec2* right = nullptr);

SkPath getSkiaPath(const FramedPath& fp);

SkPath getSkiaStrokePath(const SkPath& sp);

std::optional<Vec2> findClosePointToLine(const Vec2& m, const Vec2& p0, const Vec2& p1);

std::optional<Vec2> findClosePointToQuad(const Vec2& m,
                                         const Vec2& p0,
                                         const Vec2& p1,
                                         const Vec2& p2);

std::optional<Vec2> findClosePointToCube(const Vec2& m,
                                         const Vec2& p0,
                                         const Vec2& p1,
                                         const Vec2& p2,
                                         const Vec2& p3);

double perpendicularDistance(const Vec2& p, const Vec2& a, const Vec2& b);

std::vector<Vec2> simplifyByDouglasPeucker(const std::vector<Vec2>& pts,
                                           size_t i,
                                           size_t j,
                                           double epsilon = 1.0);

}; // namespace GeometrySystem

}; // namespace VGG

#endif // __GEOMETRY_SYSTEM_HPP__
