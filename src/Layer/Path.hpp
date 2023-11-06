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

#include "Math/Algebra.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>
#include <include/core/SkBlendMode.h>
#include <include/core/SkImageFilter.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/core/SkImage.h>
#include <include/effects/SkImageFilters.h>
#include <include/pathops/SkPathOps.h>
#include <include/core/SkFontStyle.h>
#include <limits>
#include <math.h>
#include "Layer/Core/Attrs.hpp"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/transform2.hpp"

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

void inline compute(const glm::vec2& p0,
                    const glm::vec2& p1,
                    const glm::vec2& p2,
                    float& vectorAngle,
                    int& vectorAngleSign,
                    float& normAngleFromXAxis,
                    int& normAngleSign,
                    float& len1,
                    float& len2)
{
  const auto v1 = p0 - p1;
  const auto v2 = p2 - p1;
  len1 = glm::length(v1);
  len2 = glm::length(v2);
  const auto norm = glm::normalize(p0 - p1);
  const auto norm2 = glm::normalize(p2 - p1);
  vectorAngle = glm::angle(norm, norm2);
  vectorAngleSign = VGG ::vectorSign(norm, norm2);
  normAngleFromXAxis = glm::angle(norm, glm::vec2{ 1, 0 });
  normAngleSign = VGG::vectorSign(glm::vec2{ 1, 0 }, norm);
}

inline bool addSmoothingRadiusCurveInLocalSpace(const glm::vec2& cornerPoint,
                                                float theta,
                                                int thetaSign,
                                                float signedPivotVectorAngle,
                                                float smooth,
                                                float radius,
                                                float maxConsume,
                                                float len1,
                                                float len2,
                                                SkPath& path)
{
  if (theta == 0.f)
    return false;

  const auto cosTheta = std::cos(theta);
  const auto singedTheta = thetaSign * theta;
  const auto halfVec =
    glm::normalize(glm::vec2{ std::cos(singedTheta), std::sin(singedTheta) } + glm::vec2{ 1, 0 });
  const auto arcCenter = halfVec * (radius / std::sin(theta / 2));

  const float con = radius * std::sqrt((1.f + cosTheta) / (1.f - cosTheta));

  const float maxSmoothOnV1 = len1 / con - 1.0f;
  const float maxSmoothOnV2 = len2 / con - 1.0f;
  const float smoothOnVector = std::min(maxSmoothOnV1, maxSmoothOnV2);
  smooth = std::min(std::max(smoothOnVector, 0.f), smooth);
  const float consume = (1.0f + smooth) * con;

  const auto halfArcRadian = (1.0f - smooth) * (math::number::Pi - theta) / 2.0f;
  auto pointOnCircle = [](const glm::vec2& center,
                          float radius,
                          float radianFromBisect,
                          float theta,
                          int sign) -> glm::vec2
  {
    const auto radian = sign * ((math::number::Pi - theta / 2.f) - radianFromBisect);
    return glm::vec2{ center.x + radius * std::cos(radian), center.y + radius * std::sin(radian) };
  };

  glm::vec2 first = glm::vec2{ consume, 0.0 };
  glm::vec2 last = pointOnCircle(arcCenter, radius, halfArcRadian, theta, -thetaSign);

  const auto arcNormal = arcCenter - last;
  float x;
  if (arcNormal.x != 0.f)
    x = (last.y * (arcNormal.y) + last.x * (arcNormal.x)) / (arcNormal.x);
  else
    x = last.x;
  glm::vec2 third = glm::vec2{ x, 0 };
  constexpr float PROPORTION = 2.f / 3.f; // between (0, 1)
  glm::vec2 second = glm::lerp(first, third, PROPORTION);

  glm::mat3 reflect = glm::identity<glm::mat3>();
  reflect = glm::reflect2D(reflect, glm::normalize(glm::vec3{ -arcCenter.y, arcCenter.x, 0 }));
  glm::vec2 rFirst = reflect * glm::vec3{ last, 1.f };
  glm::vec2 rSecond = reflect * glm::vec3{ third, 1.f };
  glm::vec2 rThird = reflect * glm::vec3{ second, 1.f };
  glm::vec2 rLast = reflect * glm::vec3{ first, 1.f };

  // transform all point into orignal coordiante frame
  auto toOriginal = [](const glm::vec2& translate, float rotation) -> glm::mat3
  {
    glm::mat3 invMat = glm::identity<glm::mat3>();
    invMat = glm::translate(invMat, translate);
    invMat = glm::rotate(invMat, rotation);
    return invMat;
  };

  auto invMat = toOriginal(cornerPoint, signedPivotVectorAngle);
  auto inv = [&](glm::vec2& v) { v = invMat * glm::vec3{ v, 1.0 }; };

  inv(first);
  inv(second);
  inv(third);
  inv(last);

  inv(rFirst);
  inv(rSecond);
  inv(rThird);
  inv(rLast);

  path.lineTo(first.x, first.y);
  path.cubicTo(second.x, second.y, third.x, third.y, last.x, last.y);

  if (halfArcRadian > std::numeric_limits<float>::epsilon())
  {
    auto arcPoint = arcCenter - halfVec * float(radius / std::cos(halfArcRadian));
    inv(arcPoint);
    path.arcTo(arcPoint.x, arcPoint.y, rFirst.x, rFirst.y, radius);
  }
  path.cubicTo(rSecond.x, rSecond.y, rThird.x, rThird.y, rLast.x, rLast.y);
  // cur = rLast; // update start point for next
  // DEBUG("Update Prev: [%f, %f]", cur.x, cur.y);
  return true;
}

inline bool addFigmaSquircleCurve1(const glm::vec2& prev,
                                   const glm::vec2& cur,
                                   const glm::vec2& next,
                                   float radius,
                                   float smooth,
                                   SkPath& path)
{
}

inline bool addFigmaSquircleCurve(const glm::vec2& prev,
                                  const glm::vec2& cur,
                                  const glm::vec2& next,
                                  float radius,
                                  float smooth,
                                  SkPath& path)
{
  DEBUG("------------------------");
  DEBUG("Prev: [%f, %f]", prev.x, prev.y);
  DEBUG("Cur: [%f, %f]", cur.x, cur.y);
  DEBUG("Next: [%f, %f]", next.x, next.y);
  const auto v1o = prev - cur;
  const auto v2o = next - cur;
  const auto outerProduct = glm::cross(glm::vec3{ v1o, 0 }, glm::vec3{ v2o, 0 });
  const auto sign = outerProduct.z > 0 ? 1 : (outerProduct.z < 0 ? -1 : 0);

  const auto len1 = glm::length(v1o);
  if (len1 == 0.f)
    return false;
  const auto invLen1 = 1.0 / len1;
  const auto len2 = glm::length(v2o);
  if (len2 == 0.f)
    return false;
  const auto invLen2 = 1.0 / len2;
  const auto v1on = glm::vec2{ v1o.x * invLen1, v1o.y * invLen1 };
  const auto v2on = glm::vec2{ v2o.x * invLen2, v2o.y * invLen2 };
  const auto theta = glm::angle(v1on, v2on);
  if (theta == 0.f)
    return false;

  auto toLocalFrom = [&v1on](const glm::vec2& p0,
                             const glm::vec2& p1) -> std::pair<glm::mat3, glm::mat3>
  {
    glm::mat3 mat = glm::identity<glm::mat3>();
    auto sign = VGG::vectorSign(v1on, { 1, 0 });
    auto angleFromXAxis = sign * glm::angle(v1on, glm::vec2{ 1, 0 });
    mat = glm::rotate(mat, angleFromXAxis);
    mat = glm::translate(mat, -p1);
    glm::mat3 invMat = glm::identity<glm::mat3>();
    invMat = glm::translate(invMat, p1);
    invMat = glm::rotate(invMat, -angleFromXAxis);
    return { mat, invMat };
  };

  auto transform = toLocalFrom(prev, cur);
  const auto& invMat = transform.second;
  auto inv = [&](glm::vec2& v) { v = invMat * glm::vec3{ v, 1.0 }; };
  const auto& mat = transform.first;

  // const glm::vec2 p0 = mat * glm::vec3{ prev, 1.0 };
  const glm::vec2 p2 = mat * glm::vec3{ next, 1.0 };

  // trivial constantexpr just make code readable
  constexpr auto v1n = glm::vec2{ 1, 0 }; // NOLINT
  const auto& v2 = p2;
  const auto v2n = glm::vec2{ v2.x * invLen2, v2.y * invLen2 };
  const auto halfVec = glm::normalize(v1n + v2n);
  const auto arcCenter = halfVec * (radius / std::sin(theta / 2));

  const float con = radius * std::sqrt((1.f + std::cos(theta)) / (1.f - std::cos(theta)));
  const float consume = (1.0 + smooth) * con;
  const float maxSmoothOnV1 = len1 / con - 1.0f;
  const float maxSmoothOnV2 = len2 / con - 1.0f;
  const float smoothOnVector = std::min(maxSmoothOnV1, maxSmoothOnV2);

  smooth = std::min(std::max(smoothOnVector, 0.f), smooth);
  if (smooth == 0.f)
  {
    path.arcTo(cur.x, cur.y, next.x, next.y, radius);
    // SkPoint lastPoint;
    // path.getLastPt(&lastPoint);
    // cur.x = lastPoint.x();
    // cur.y = lastPoint.y();
    return true;
  }

  const auto halfArcRadian = (1.0f - smooth) * (math::number::Pi - theta) / 2.0f;

  auto pointOnCircle =
    [sign](const glm::vec2& center, float radius, float radianFromBisect) -> glm::vec2
  {
    const auto v1 = -center;
    constexpr auto v2 = glm::vec2{ 1, 0 }; // NOLINT
    const auto radian = -sign * (glm::angle(glm::normalize(v1), v2) - radianFromBisect);
    return glm::vec2{ center.x + radius * std::cos(radian), center.y + radius * std::sin(radian) };
  };

  glm::vec2 first = glm::vec2{ consume, 0.0 };
  glm::vec2 last = pointOnCircle(arcCenter, radius, halfArcRadian);

  const auto arcNormal = arcCenter - last;
  float toX;
  if (arcNormal.x != 0.f)
  {
    toX = (last.y * (arcNormal.y) + last.x * (arcNormal.x)) / (arcNormal.x);
  }
  else
  {
    toX = last.x;
  }
  glm::vec2 third = glm::vec2{ toX, 0 };

  constexpr float PROPORTION = 2.f / 3.f; // between (0, 1)
  glm::vec2 second = glm::lerp(first, third, PROPORTION);

  glm::mat3 reflect = glm::identity<glm::mat3>();
  reflect = glm::reflect2D(reflect, glm::normalize(glm::vec3{ -arcCenter.y, arcCenter.x, 0 }));
  glm::vec2 rFirst = reflect * glm::vec3{ last, 1.f };
  glm::vec2 rSecond = reflect * glm::vec3{ third, 1.f };
  glm::vec2 rThird = reflect * glm::vec3{ second, 1.f };
  glm::vec2 rLast = reflect * glm::vec3{ first, 1.f };

  // transform all point into orignal coordiante frame
  inv(first);
  inv(second);
  inv(third);
  inv(last);

  inv(rFirst);
  inv(rSecond);
  inv(rThird);
  inv(rLast);

  path.lineTo(first.x, first.y);
  path.cubicTo(second.x, second.y, third.x, third.y, last.x, last.y);

  if (halfArcRadian > std::numeric_limits<float>::epsilon())
  {
    auto arcPoint = arcCenter - halfVec * float(radius / std::cos(halfArcRadian));
    inv(arcPoint);
    path.arcTo(arcPoint.x, arcPoint.y, rFirst.x, rFirst.y, radius);
  }
  path.cubicTo(rSecond.x, rSecond.y, rThird.x, rThird.y, rLast.x, rLast.y);
  // cur = rLast; // update start point for next
  DEBUG("Update Prev: [%f, %f]", cur.x, cur.y);
  return true;
}

inline SkPath makePath(const Contour& contour)
{
  const auto& points = contour;
  auto isClosed = contour.closed;
  auto roundedSmooth = contour.cornerSmooth;
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
        path.arcTo(cp.point.x, cp.point.y, np.point.x, np.point.y, cp.radius);
      }
      else
      {
        addFigmaSquircleCurve1(pp->point, cp.point, np.point, cp.radius, roundedSmooth, path);
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

  if (roundedSmooth <= 0.f)
  {
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
      {
        consumed = peek1(&buffer[pp], buffer[cp], path);
      }
      else if (buffer[cp].radius > 0)
      {
        consumed = peek2(&buffer[pp], buffer[cp], points[next], roundedSmooth, path);
      }
      segments -= consumed;

      const auto total = points.size();

      prev = (prev + consumed) % total;
      cur = (cur + consumed) % total;
      next = (next + consumed) % total;

      // swap buffer
      pp = cp;
      cp = 1 - cp;
      buffer[cp] = points[cur];
    }
  }
  else
  {
    if (contour.allStraightLines())
    {
      DEBUG("polygon");
      // for smoothing corner, only polygon is supported
      if (!isClosed || buffer[pp].radius <= 0)
      {
        path.moveTo(buffer[pp].point.x, buffer[pp].point.y);
      }
      else // handle the special case that a closed path begin with a rounded point
      {
        // TODO::update buffer[pp]
        path.moveTo(buffer[pp].point.x, buffer[pp].point.y);
      }
      size_t prev2 = points.size() - 1;
      size_t next2 = (next + 1) % points.size();
      const auto total = points.size();
      while (segments > 0)
      {
        int consumed = 0;
        if (buffer[cp].radius <= 0)
        {
          consumed = peek1(&buffer[pp], buffer[cp], path);
        }
        else if (buffer[cp].radius > 0)
        {
          float theta;
          int thetaSign;
          float pivotAngle;
          int pivotSign;
          const auto prevPrevPoint = points[prev2].point;
          const auto prevPoint = buffer[pp].point;
          const auto curPoint = buffer[cp].point;
          const auto nextPoint = points[next].point;
          const auto nextNextPoint = points[next2].point;
          const auto radius = buffer[cp].radius;
          float len1, len2;
          compute(prevPoint,
                  curPoint,
                  nextPoint,
                  theta,
                  thetaSign,
                  pivotAngle,
                  pivotSign,
                  len1,
                  len2);

          const auto prevAngle = glm::angle(glm::normalize(prevPrevPoint - prevPoint),
                                            glm::normalize(curPoint - prevPoint));
          const auto curAngle = theta;
          const auto nextAngle = glm::angle(glm::normalize(nextNextPoint - nextPoint),
                                            glm::normalize(curPoint - nextPoint));

          len1 = (prevAngle * len1) / (prevAngle + curAngle);
          len2 = (nextAngle * len2) / (nextAngle + curAngle);
          len1 = 10;
          len2 = 10;
          DEBUG("%f %f %f", prevAngle, curAngle, nextAngle);
          addSmoothingRadiusCurveInLocalSpace(curPoint,
                                              theta,
                                              thetaSign,
                                              pivotSign * pivotAngle,
                                              roundedSmooth,
                                              points[cp].radius,
                                              1.0,
                                              len1,
                                              len2,
                                              path);
          consumed = 1;
        }
        segments -= consumed;

        prev2 = (prev2 + consumed) % total;
        prev = (prev + consumed) % total;
        cur = (cur + consumed) % total;
        next = (next + consumed) % total;
        next2 = (next + consumed) % total;

        // swap buffer
        pp = cp;
        cp = 1 - cp;
        buffer[cp] = points[cur];
      }
    }
    else
    {
      DEBUG("Dont supported smoothing corner for non polygon");
    }
  }

  if (isClosed)
  {
    path.close();
  }
  return path;
};
} // namespace VGG::layer
