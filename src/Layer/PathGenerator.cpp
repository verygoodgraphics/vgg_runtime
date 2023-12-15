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
#include "PathGenerator.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Renderer.hpp"
#include "Math/Math.hpp"
#include "Utility/VggFloat.hpp"

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>
#include <variant>

namespace
{

struct CubicCurve
{
  glm::vec2 p1;
  glm::vec2 c1;
  glm::vec2 c2;
  glm::vec2 p2;
};

struct QuadricCurve
{
  glm::vec2 p1;
  glm::vec2 c1;
  glm::vec2 p2;
};

struct Line
{
  glm::vec2 p1;
  glm::vec2 p2;
};

struct ArcCurve
{
  glm::vec2 p1;
  glm::vec2 p2;
  float     radius{ 0.f };
};

using FullSmooth = std::pair<CubicCurve, CubicCurve>;
using PartialSmooth = std::tuple<CubicCurve, ArcCurve, CubicCurve>;
using SmoothResult = std::variant<FullSmooth, PartialSmooth>;

using NoRadiusCurveResult = std::variant<CubicCurve, QuadricCurve, Line>;
using RadiusCurveResult = std::variant<CubicCurve, QuadricCurve, ArcCurve, Line>;
using SmoothCurveResult = std::variant<SmoothResult, ArcCurve, RadiusCurveResult>;

void inline computeAdjacent3PointsInfo(
  const glm::vec2& p0,
  const glm::vec2& p1,
  const glm::vec2& p2,
  float&           vectorAngle,
  int&             vectorAngleSign,
  float&           normAngleFromXAxis,
  int&             normAngleSign,
  float&           len1,
  float&           len2)
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

bool inline computeAdjacent5PointsInfo(
  const ControlPoint& prevPrevPoint,
  const ControlPoint& prevPoint,
  const ControlPoint& curPoint,
  const ControlPoint& nextPoint,
  const ControlPoint& nextNextPoint,
  float&              theta,
  int&                thetaSign,
  float&              pivotAngle,
  int&                pivotSign,
  float&              len1,
  float&              len2)
{
  // auto radius = curPoint.radius;
  computeAdjacent3PointsInfo(
    prevPoint.point,
    curPoint.point,
    nextPoint.point,
    theta,
    thetaSign,
    pivotAngle,
    pivotSign,
    len1,
    len2);

  auto halfCot = [](float& a) { a = 1.0 / std::tan(a / 2); };
  auto prevAngle = glm::angle(
    glm::normalize(prevPrevPoint.point - prevPoint.point),
    glm::normalize(curPoint.point - prevPoint.point));
  auto curAngle = theta;
  auto nextAngle = glm::angle(
    glm::normalize(nextNextPoint.point - nextPoint.point),
    glm::normalize(curPoint.point - nextPoint.point));

  if (
    floatNearlyEqual(prevAngle, math::number::Pi) || floatNearlyEqual(curAngle, math::number::Pi) ||
    floatNearlyEqual(nextAngle, math::number::Pi))
  {
    return false;
  }
  halfCot(prevAngle);
  halfCot(curAngle);
  halfCot(nextAngle);
  // const auto halfTanTheta = std::tan(theta / 2.f);
  len1 = (curAngle * len1) / (prevAngle + curAngle);
  len2 = (curAngle * len2) / (nextAngle + curAngle);
  return false;
}

std::optional<SmoothResult> addSmoothingRadiusCurveInLocalSpace(
  const glm::vec2& point,
  float            theta,
  int              thetaSign,
  float            signedPivotVectorAngle,
  float            smooth,
  float            radius,
  float            maxLength)
{
  // const auto cosTheta = std::cos(theta);
  const auto singedTheta = thetaSign * theta;
  const auto halfSinTheta = std::sin(theta / 2.f);
  const auto halfTanTheta = std::tan(theta / 2.f);
  radius = std::min(maxLength * halfTanTheta, radius);
  const auto halfVec =
    glm::normalize(glm::vec2{ std::cos(singedTheta), std::sin(singedTheta) } + glm::vec2{ 1, 0 });
  const auto  arcCenter = halfVec * (radius / halfSinTheta);
  float       roundedLength = radius / halfTanTheta;
  const float maxSmooth = std::max(maxLength / roundedLength - 1.0f, 0.f);
  smooth = std::min(maxSmooth, smooth);

  auto toOriginal = [](const glm::vec2& translate, float rotation) -> glm::mat3
  {
    glm::mat3 invMat = glm::identity<glm::mat3>();
    invMat = glm::translate(invMat, translate);
    invMat = glm::rotate(invMat, rotation);
    return invMat;
  };
  if (smooth <= 0.f)
  {
    return std::nullopt;
  }

  auto invMat = toOriginal(point, signedPivotVectorAngle);
  auto inv = [&](glm::vec2& v) { v = invMat * glm::vec3{ v, 1.0 }; };

  const float consume = (1.0f + smooth) * roundedLength;

  const auto halfArcRadian = (1.0f - smooth) * (math::number::Pi - theta) / 2.0f;
  auto       pointOnCircle =
    [](const glm::vec2& center, float radius, float radianFromBisect, float theta, int sign)
    -> glm::vec2
  {
    const auto radian = sign * ((math::number::Pi - theta / 2.f) - radianFromBisect);
    return glm::vec2{ center.x + radius * std::cos(radian), center.y + radius * std::sin(radian) };
  };

  glm::vec2 first = glm::vec2{ consume, 0.0 };
  glm::vec2 last = pointOnCircle(arcCenter, radius, halfArcRadian, theta, -thetaSign);

  const auto arcNormal = arcCenter - last;
  float      x;
  if (arcNormal.x != 0.f)
    x = (last.y * (arcNormal.y) + last.x * (arcNormal.x)) / (arcNormal.x);
  else
    x = last.x;
  glm::vec2       third = glm::vec2{ x, 0 };
  constexpr float PROPORTION = 2.f / 3.f; // between (0, 1)
  glm::vec2       second = glm::lerp(first, third, PROPORTION);

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

  CubicCurve seg1{ first, second, third, last };
  CubicCurve seg2{ rFirst, rSecond, rThird, rLast };

  if (halfArcRadian > std::numeric_limits<float>::epsilon())
  {
    auto arcPoint = arcCenter - halfVec * float(radius / std::cos(halfArcRadian));
    inv(arcPoint);
    ArcCurve arc{ arcPoint, rFirst, radius };
    return PartialSmooth{ seg1, arc, seg2 };
  }
  return FullSmooth{ seg1, seg2 };
}

std::pair<int, NoRadiusCurveResult> curveWithNoRadius(
  const ControlPoint* pp,
  const ControlPoint& cp)
{
  const auto prevHasFrom = pp->from.has_value();
  const auto curHasTo = cp.to.has_value();
  if (prevHasFrom && curHasTo)
    return { 1, CubicCurve{ pp->point, *pp->from, *cp.to, cp.point } };
  else if (prevHasFrom && !curHasTo)
    return { 1, QuadricCurve{ pp->point, pp->from.value(), cp.point } };
  else if (!prevHasFrom && curHasTo)
    return { 1, QuadricCurve{ pp->point, cp.to.value(), cp.point } };
  else if (!prevHasFrom && !curHasTo)
    return { 1, Line{ pp->point, cp.point } };
  return { 1, NoRadiusCurveResult{} };
}

// No smooth
std::pair<int, RadiusCurveResult> curveWithRadius(
  const ControlPoint& prevPrevPoint,
  const ControlPoint* pp,
  const ControlPoint& cp,
  const ControlPoint& np,
  const ControlPoint& nextNextPoint)
{
  const auto prevHasFrom = pp->from.has_value();
  const auto curHasTo = cp.to.has_value();
  // const auto nextHasFrom = np.from.has_value();
  const auto nextHasTo = np.to.has_value();
  const auto curHasFrom = cp.from.has_value();

  auto eval = [](const ControlPoint& c1, const ControlPoint& c2, const ControlPoint& c3)
  {
    if (c2.radius <= 0)
      return 0.f;
    const auto& p1 = c1.point;
    const auto& p2 = c2.point;
    const auto& p3 = c3.point;
    auto        halfCot = [](float& a) { return (float)1.0 / std::tan(a / 2); };
    // auto        n1 = glm::normalize(p1 - p2);
    // auto        n2 = glm::normalize(p3 - p2);
    auto        prevAngle = glm::angle(glm::normalize(p2 - p1), glm::normalize(p2 - p3));
    // float       len = halfCot(prevAngle) * c2.radius;
    return halfCot(prevAngle);
  };
  // For the point with radius, just deal with the point between two straight lines now
  if (!prevHasFrom && !curHasTo && !curHasFrom && !nextHasTo)
  {
    auto l1 = glm::distance(cp.point, pp->point);
    auto l2 = glm::distance(cp.point, np.point);
    auto r1 = eval(prevPrevPoint, *pp, cp);
    auto r2 = eval(*pp, cp, np);
    ASSERT(r2 > 0);
    auto     r3 = eval(cp, np, nextNextPoint);
    auto     maxRadius = std::min(r2 / (r1 + r2) * l1, r2 / (r2 + r3) * l2);
    ArcCurve arcCurve{ cp.point, np.point, std::min(maxRadius, cp.radius) };
    return { 1, arcCurve };
  }
  auto [consume, res] = curveWithNoRadius(pp, cp);
  if (auto r = std::get_if<CubicCurve>(&res); r)
    return { consume, *r };
  else if (auto r = std::get_if<QuadricCurve>(&res); r)
    return { consume, *r };
  else if (auto r = std::get_if<Line>(&res); r)
    return { consume, *r };
  ASSERT(false);
  return {};
}

void calcPointsFromCircle(
  const glm::vec2& p1,
  const glm::vec2& p2,
  const glm::vec2& p3,
  float            radius,
  glm::vec2&       first,
  glm::vec2&       second)
{
  auto  halfCot = [](float& a) { return 1.0 / std::tan(a / 2); };
  auto  n1 = glm::normalize(p1 - p2);
  auto  n2 = glm::normalize(p3 - p2);
  auto  prevAngle = glm::angle(glm::normalize(p2 - p1), glm::normalize(p2 - p3));
  float len = halfCot(prevAngle) * radius;
  first = p2 + n1 * len;
  second = p2 + n2 * len;
}

std::pair<int, SmoothCurveResult> smoothCurveWithRadius(
  const ControlPoint& prevPrevPoint,
  const ControlPoint& prevPoint,
  const ControlPoint& curPoint,
  const ControlPoint& nextPoint,
  const ControlPoint& nextNextPoint,
  float               smooth)
{
  const auto prevHasFrom = prevPoint.from.has_value();
  const auto curHasTo = curPoint.to.has_value();
  const auto nextHasTo = nextPoint.to.has_value();
  const auto curHasFrom = curPoint.from.has_value();
  if (!prevHasFrom && !curHasTo && !curHasFrom && !nextHasTo)
  {
    // just deal with smoothed radius curved between two straight lines
    float theta;
    int   thetaSign;
    float pivotAngle;
    int   pivotSign;
    float len1, len2;
    auto  radius = curPoint.radius;
    auto  smoothable = computeAdjacent5PointsInfo(
      prevPrevPoint,
      prevPoint,
      curPoint,
      nextPoint,
      nextNextPoint,
      theta,
      thetaSign,
      pivotAngle,
      pivotSign,
      len1,
      len2);
    const auto  halfTanTheta = std::tan(theta / 2.f);
    const float maxLength = std::min(len1, len2);
    if (smoothable)
    {
      auto result = addSmoothingRadiusCurveInLocalSpace(
        curPoint.point,
        theta,
        thetaSign,
        pivotSign * pivotAngle,
        smooth,
        radius,
        maxLength);
      if (result)
        return { 1, *result };
      smoothable = false;
    }
    if (!smoothable)
    {
      radius = std::min(maxLength * halfTanTheta, radius);
      ArcCurve arcCurve{ curPoint.point, nextPoint.point, radius };
      // SkPoint  lastPt;
      return { 1, arcCurve };
    }
    return { 1, SmoothCurveResult{} };
  }
  else
  {
    return curveWithRadius(prevPrevPoint, &prevPoint, curPoint, nextPoint, nextNextPoint);
  }
}

} // namespace

namespace VGG::layer
{
SkPath makePath(const Contour& contour)
{
  const auto& points = contour;
  auto        isClosed = contour.closed;
  auto        smooth = contour.cornerSmooth;
  if (points.size() < 2)
  {
    WARN("Too few path points.");
    return {};
  }
  SkPath     path;
  const auto total = points.size();
  size_t     prev = 0;
  size_t     cur = 1;
  size_t     next = (cur + 1) % total;
  size_t     prev2 = points.size() - 1;
  size_t     next2 = (next + 1) % total;

  int          segments = isClosed ? total : total - 1;
  ControlPoint buffer[2] = { points[prev], points[cur] };
  int          cp = 1;
  int          pp = 1 - cp;

  auto resolveNoRadiusCurveResult = [&](const NoRadiusCurveResult& result)
  {
    std::visit(
      Overloaded{ [&](const CubicCurve& c)
                  { path.cubicTo(c.c1.x, c.c1.y, c.c2.x, c.c2.y, c.p2.x, c.p2.y); },
                  [&](const QuadricCurve& c) { path.quadTo(c.c1.x, c.c1.y, c.p2.x, c.p2.y); },
                  [&](const Line& l) { path.lineTo(l.p2.x, l.p2.y); } },
      result);
  };

  auto resolveRadiusCurveResult = [&](const RadiusCurveResult& result)
  {
    std::visit(
      Overloaded{ [&](const CubicCurve& c)
                  { path.cubicTo(c.c1.x, c.c1.y, c.c2.x, c.c2.y, c.p2.x, c.p2.y); },
                  [&](const QuadricCurve& c) { path.quadTo(c.c1.x, c.c1.y, c.p2.x, c.p2.y); },
                  [&](const ArcCurve& c) { path.arcTo(c.p1.x, c.p1.y, c.p2.x, c.p2.y, c.radius); },
                  [&](const Line& l) { path.lineTo(l.p2.x, l.p2.y); } },
      result);
  };

  auto resolveSmoothCurveResult = [&](const SmoothCurveResult& result)
  {
    std::visit(
      Overloaded{ [&](const SmoothResult& c)
                  {
                    if (auto smooth = std::get_if<PartialSmooth>(&c); smooth)
                    {
                      const auto& [c1, arc, c2] = *smooth;
                      path.lineTo(c1.p1.x, c1.p1.y);
                      path.cubicTo(c1.c1.x, c1.c1.y, c1.c2.x, c1.c2.y, c1.p2.x, c1.p2.y);
                      path.arcTo(arc.p1.x, arc.p1.y, arc.p2.x, arc.p2.y, arc.radius);
                      path.cubicTo(c2.c1.x, c2.c1.y, c2.c2.x, c2.c2.y, c2.p2.x, c2.p2.y);
                    }
                    else if (auto smooth = std::get_if<FullSmooth>(&c); smooth)
                    {
                      const auto& [c1, c2] = *smooth;
                      path.lineTo(c1.p1.x, c1.p1.y);
                      path.cubicTo(c1.c1.x, c1.c1.y, c1.c2.x, c1.c2.y, c1.p2.x, c1.p2.y);
                      path.cubicTo(c2.c1.x, c2.c1.y, c2.c2.x, c2.c2.y, c2.p2.x, c2.p2.y);
                    }
                  },
                  [&](const ArcCurve& c) { path.arcTo(c.p1.x, c.p1.y, c.p2.x, c.p2.y, c.radius); },
                  [&](const RadiusCurveResult& result) { resolveRadiusCurveResult(result); } },
      result);
  };

  if (smooth <= 0)
  {
    if (buffer[pp].radius <= 0.f)
      path.moveTo(buffer[pp].point.x, buffer[pp].point.y);
    else
    {
      const auto& prevPrevPoint = points[(-2 + total) % total];
      const auto& prevPoint = points[(-1 + total) % total];
      const auto& curPoint = points[0];
      const auto& nextPoint = points[1 % total];
      const auto& nextNextPoint = points[2 % total];
      const auto& [_, curve] =
        curveWithRadius(prevPrevPoint, &prevPoint, curPoint, nextPoint, nextNextPoint);
      std::visit(
        Overloaded{ [&](const CubicCurve& c) { path.moveTo(c.p2.x, c.p2.y); },
                    [&](const QuadricCurve& c) { path.moveTo(c.p2.x, c.p2.y); },
                    [&](const ArcCurve& c)
                    {
                      glm::vec2 first, second;
                      calcPointsFromCircle(prevPoint.point, c.p1, c.p2, c.radius, first, second);
                      path.moveTo(second.x, second.y);
                    },
                    [&](const Line& l) { path.moveTo(l.p2.x, l.p2.y); } },
        curve);
    }
    while (segments > 0)
    {
      int         consumed = 0;
      const auto& prevPoint = buffer[pp];
      const auto& curPoint = buffer[cp];
      const auto& nextPoint = points[next];
      const auto& prevPrevPoint = points[prev2];
      const auto& nextNextPoint = points[next2];
      if (buffer[cp].radius <= 0)
      {
        const auto& [c, result] = curveWithNoRadius(&prevPoint, curPoint);
        resolveNoRadiusCurveResult(result);
        consumed = c;
      }
      else
      {
        const auto& [c, result] =
          curveWithRadius(prevPrevPoint, &prevPoint, curPoint, nextPoint, nextNextPoint);
        resolveRadiusCurveResult(result);
        consumed = c;
      }
      segments -= consumed;
      prev2 = (prev2 + consumed) % total;
      prev = (prev + consumed) % total;
      cur = (cur + consumed) % total;
      next = (next + consumed) % total;
      next2 = (next2 + consumed) % total;
      // swap buffer
      pp = cp;
      cp = 1 - cp;
      buffer[cp] = points[cur];
    }
  }
  else
  {
    if (points[0].radius <= 0.f)
    {
      path.moveTo(points[0].point.x, points[0].point.y);
    }
    else
    {
      const auto& prevPrevPoint = points[(-2 + total) % total];
      const auto& prevPoint = points[(-1 + total) % total];
      const auto& curPoint = points[0];
      const auto& nextPoint = points[1 % total];
      const auto& nextNextPoint = points[2 % total];
      auto        res =
        smoothCurveWithRadius(prevPrevPoint, prevPoint, curPoint, nextPoint, nextNextPoint, smooth)
          .second;
      std::visit(
        Overloaded{
          [&](const SmoothResult& c)
          {
            if (auto smooth = std::get_if<PartialSmooth>(&c); smooth)
            {
              const auto& [cubic1, arc, cubic2] = *smooth;
              path.moveTo(cubic2.p2.x, cubic2.p2.y);
            }
            else if (auto smooth = std::get_if<FullSmooth>(&c); smooth)
            {
              const auto& [cubic1, cubic2] = *smooth;
              path.moveTo(cubic2.p2.x, cubic2.p2.y);
            }
          },
          [&](const ArcCurve& c)
          {
            glm::vec2 first, second;
            calcPointsFromCircle(prevPoint.point, c.p1, c.p2, c.radius, first, second);
            path.moveTo(second.x, second.y);
          },

          [&](const RadiusCurveResult& result)
          {
            std::visit(
              Overloaded{
                [&](const CubicCurve& c) { path.moveTo(c.p2.x, c.p2.y); },
                [&](const QuadricCurve& c) { path.moveTo(c.p2.x, c.p2.y); },
                [&](const ArcCurve& c)
                {
                  glm::vec2 first, second;
                  calcPointsFromCircle(prevPoint.point, c.p1, c.p2, c.radius, first, second);
                  path.moveTo(second.x, second.y);
                },
                [&](const Line& l) { path.moveTo(l.p2.x, l.p2.y); } },
              result);
          } },
        res);
    }
    while (segments > 0)
    {
      int         consumed = 0;
      const auto& prevPoint = buffer[pp];
      const auto& curPoint = buffer[cp];
      const auto& nextPoint = points[next];
      const auto& prevPrevPoint = points[prev2];
      const auto& nextNextPoint = points[next2];
      if (buffer[cp].radius <= 0)
      {
        const auto& [c, result] = curveWithNoRadius(&prevPoint, curPoint);
        resolveNoRadiusCurveResult(result);
        consumed = c;
      }
      else
      {
        const auto& [c, result] = smoothCurveWithRadius(
          prevPrevPoint,
          prevPoint,
          curPoint,
          nextPoint,
          nextNextPoint,
          smooth);
        resolveSmoothCurveResult(result);
        consumed = c;
      }
      segments -= consumed;
      prev2 = (prev2 + consumed) % total;
      prev = (prev + consumed) % total;
      cur = (cur + consumed) % total;
      next = (next + consumed) % total;
      next2 = (next2 + consumed) % total;
      // swap buffer
      pp = cp;
      cp = 1 - cp;
      buffer[cp] = points[cur];
    }
  }
  if (isClosed)
  {
    path.close();
  }
  return path;
};
} // namespace VGG::layer
