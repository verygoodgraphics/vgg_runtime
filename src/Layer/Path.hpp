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

#include "Utility/Log.hpp"

#include <include/core/SkBlendMode.h>
#include <include/core/SkImageFilter.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/core/SkImage.h>
#include <include/effects/SkImageFilters.h>
#include <include/pathops/SkPathOps.h>
#include <include/core/SkFontStyle.h>
#include "Layer/Core/Attrs.hpp"

namespace VGG::layer
{

inline double calcRadius(double r0,
                         const glm::vec2& p0,
                         const glm::vec2& p1,
                         const glm::vec2& p2,
                         glm::vec2* left,
                         glm::vec2* right)
{
  constexpr float EPS = std::numeric_limits<float>::epsilon();
  glm::vec2 a = p0 - p1;
  glm::vec2 b = p2 - p1;
  double alen = glm::distance(p0, p1);
  double blen = glm::distance(p2, p1);
  if (std::fabs(alen) < EPS || std::fabs(blen) < EPS)
  {
    return 0.;
  }
  ASSERT(alen > 0 && blen > 0);
  double cosTheta = glm::dot(a, b) / alen / blen;
  if (cosTheta + 1 < EPS) // cosTheta == -1
  {
    if (left)
    {
      left->x = p1.x;
      left->y = p1.y;
    }
    if (right)
    {
      right->x = p1.x;
      right->y = p1.y;
    }
    return r0;
  }
  else if (1 - cosTheta < EPS) // cosTheta == 1
  {
    return 0.;
  }
  double tanHalfTheta = std::sqrt((1 - cosTheta) / (1 + cosTheta));
  double radius = r0;
  radius = std::min(radius, 0.5 * alen * tanHalfTheta);
  radius = std::min(radius, 0.5 * blen * tanHalfTheta);
  if (left)
  {
    ASSERT(tanHalfTheta > 0);
    float len = radius / tanHalfTheta;
    *left = p1 + float(len / alen) * a;
  }
  if (right)
  {
    ASSERT(tanHalfTheta > 0);
    double len = radius / tanHalfTheta;
    *right = p1 + (float(len / blen) * b);
  }
  return radius;
}

inline glm::vec2 symmetric(const glm::vec2& point, const glm::vec2& start, const glm::vec2& end)
{
  return {};
}

// add 3 curve segments into path: cubic curve + arc + cubic curve
inline void addFigmaSquicleCurve(const glm::vec2& start,
                                 const glm::vec2& join,
                                 const glm::vec2& end,
                                 float radius,
                                 float smooth,
                                 SkPath& path)
{
  const auto theta = 0;
  auto d = [](float theta, float xi, float radius) {};
  auto c = [](float theta, float xi, float radius) {};
  auto b = [](float theta, float xi, float radius) {};

  auto maxSmooth = [](float len1, float len2, float smooth) -> float { return smooth; };

  const auto len1 = (join - start).length();
  const auto len2 = (end - join).length();

  smooth = maxSmooth(len1, len2, smooth);

  // TODO::
  glm::vec2 begin;
  glm::vec2 from;
  glm::vec2 to;
  glm::vec2 tangentedPoint;
}

inline SkPath makePath(const std::vector<PointAttr>& points, bool isClosed, float roundedSmooth)
{
  auto peek1 = [](const PointAttr* pp, const PointAttr& cp, SkPath& path) -> int
  {
    const auto prevHasFrom = pp->from.has_value();
    const auto curHasTo = cp.to.has_value();
    if (prevHasFrom && curHasTo)
      path.cubicTo(pp->from->x, pp->from->y, cp.to->x, cp.to->y, cp.point.x, cp.point.y);
    else if (prevHasFrom && !curHasTo)
      path.quadTo(pp->from->x, pp->from->y, cp.point.x, cp.point.y);
    else if (!prevHasFrom && curHasTo)
      path.quadTo(cp.to->x, cp.to->y, cp.point.x, cp.point.y);
    else if (!prevHasFrom && !curHasTo)
      path.lineTo(cp.point.x, cp.point.y);
    return 1;
  };

  auto peek2 = [&peek1](const PointAttr* pp,
                        PointAttr& cp,
                        const PointAttr& np,
                        float roundedSmooth,
                        SkPath& path) -> int
  {
    const auto prevHasFrom = pp->from.has_value();
    const auto curHasTo = cp.to.has_value();
    const auto nextHasFrom = np.from.has_value();
    const auto nextHasTo = np.to.has_value();
    const auto curHasFrom = cp.from.has_value();
    int c = 0;
    if (!prevHasFrom && !curHasTo && !curHasFrom && !nextHasTo)
    {
      // two straight lines
      if (roundedSmooth <= 0)
      {
        double r = calcRadius(cp.radius, np.point, cp.point, np.point, 0, 0);
        path.arcTo(cp.point.x, cp.point.y, np.point.x, np.point.y, r);
      }
      else
      {
        addFigmaSquicleCurve(pp->point, cp.point, np.point, cp.radius, 100, path);
      }
      return 1;
    }
    else
    {
      // cp could be modified
      WARN("Rounded point by two curve is not implemented");
      c += peek1(pp, cp, path);
      c += peek1(pp, np, path);
    }
    return c;
  };
  if (points.size() < 2)
  {
    WARN("Too few path points.");
    return {};
  }

  SkPath path;
  size_t prev = 0;
  size_t cur = 1;
  size_t next = cur + 1;

  int segments = isClosed ? points.size() : points.size() - 1;
  PointAttr buffer[2] = { points[prev], points[cur] };

  int cp = 1;
  int pp = 1 - cp;

  if (!isClosed || buffer[pp].radius <= 0)
  {
    path.moveTo(buffer[pp].point.x, buffer[pp].point.y);
  }
  else // handle the special case that a closed path begin with a rounded point
  {
    // the start point should be calculated first as the rounded curve's end point
    // TODO::update buffer[pp]
    path.moveTo(buffer[pp].point.x, buffer[pp].point.y);
  }

  while (segments > 0)
  {
    int consumed = 0;
    if (buffer[cp].radius <= 0)
      consumed = peek1(&buffer[pp], buffer[cp], path);
    else
      consumed = peek2(&buffer[pp], buffer[cp], points[next], roundedSmooth, path);
    segments -= consumed;
    prev = cur;
    cur = next;
    next = (next + consumed) % points.size();

    // swap buffer
    pp = cp;
    cp = 1 - cp;
    buffer[cp] = points[cur];
  }
  if (isClosed)
  {
    path.close();
  }
  return path;
};
} // namespace VGG::layer
