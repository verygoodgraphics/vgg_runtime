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
#include "Layer/Renderer.hpp"

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

namespace
{

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

void inline computeAdjacent5PointsInfo(
  const PointAttr& prevPrevPoint,
  const PointAttr& prevPoint,
  const PointAttr& curPoint,
  const PointAttr& nextPoint,
  const PointAttr& nextNextPoint,
  float&           theta,
  int&             thetaSign,
  float&           pivotAngle,
  int&             pivotSign,
  float&           len1,
  float&           len2)
{
  auto radius = curPoint.radius;
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

  halfCot(prevAngle);
  halfCot(curAngle);
  halfCot(nextAngle);
  const auto halfTanTheta = std::tan(theta / 2.f);
  len1 = (curAngle * len1) / (prevAngle + curAngle);
  len2 = (curAngle * len2) / (nextAngle + curAngle);
}
inline bool addSmoothingRadiusCurveInLocalSpace(
  const glm::vec2& point,
  float            theta,
  int              thetaSign,
  float            signedPivotVectorAngle,
  float            smooth,
  float            radius,
  float            maxLength,
  SkPath&          path)
{
  const auto cosTheta = std::cos(theta);
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
    return false;
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

  path.lineTo(first.x, first.y);
  path.cubicTo(second.x, second.y, third.x, third.y, last.x, last.y);
  if (halfArcRadian > std::numeric_limits<float>::epsilon())
  {
    auto arcPoint = arcCenter - halfVec * float(radius / std::cos(halfArcRadian));
    inv(arcPoint);
    path.arcTo(arcPoint.x, arcPoint.y, rFirst.x, rFirst.y, radius);
  }
  path.cubicTo(rSecond.x, rSecond.y, rThird.x, rThird.y, rLast.x, rLast.y);
  return true;
}

int peek1Point(const PointAttr* pp, const PointAttr& cp, SkPath& path)
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
}

int peek5Points(
  const PointAttr& prevPrevPoint,
  const PointAttr& prevPoint,
  const PointAttr& curPoint,
  const PointAttr& nextPoint,
  const PointAttr& nextNextPoint,
  float            smooth,
  SkPath&          path)
{
  const auto prevHasFrom = prevPoint.from.has_value();
  const auto curHasTo = curPoint.to.has_value();
  const auto nextHasFrom = nextPoint.from.has_value();
  const auto nextHasTo = nextPoint.to.has_value();
  const auto curHasFrom = curPoint.from.has_value();
  if (!prevHasFrom && !curHasTo && !curHasFrom && !nextHasTo)
  {
    float theta;
    int   thetaSign;
    float pivotAngle;
    int   pivotSign;
    float len1, len2;
    auto  radius = curPoint.radius;
    computeAdjacent5PointsInfo(
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
    const auto halfTanTheta = std::tan(theta / 2.f);
    if (smooth <= 0)
    {
      radius = std::min(std::min(halfTanTheta * len1, halfTanTheta * len2), radius);
      path.arcTo(curPoint.point.x, curPoint.point.y, nextPoint.point.x, nextPoint.point.y, radius);
    }
    else
    {
      const float maxLength = std::min(len1, len2);

      auto added = addSmoothingRadiusCurveInLocalSpace(
        curPoint.point,
        theta,
        thetaSign,
        pivotSign * pivotAngle,
        smooth,
        radius,
        maxLength,
        path);

      if (!added)
      {
        radius = std::min(maxLength * halfTanTheta, radius);
        path
          .arcTo(curPoint.point.x, curPoint.point.y, nextPoint.point.x, nextPoint.point.y, radius);
      }
    }
    return 1;
  }
  else
  {
    peek1Point(&prevPoint, curPoint, path);
    return 1;
  }
  return 1;
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

  int       segments = isClosed ? total : total - 1;
  PointAttr buffer[2] = { points[prev], points[cur] };
  int       cp = 1;
  int       pp = 1 - cp;

  path.moveTo(buffer[pp].point.x, buffer[pp].point.y);
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
      consumed = peek1Point(&prevPoint, curPoint, path);
    }
    else if (buffer[cp].radius > 0)
    {
      consumed =
        peek5Points(prevPrevPoint, prevPoint, curPoint, nextPoint, nextNextPoint, smooth, path);
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
  if (isClosed)
  {
    path.close();
  }
  return path;
};
} // namespace VGG::layer
