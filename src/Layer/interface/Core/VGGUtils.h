#pragma once

#include "VGGType.h"
#include "Geometry.hpp"

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "core/SkCanvas.h"
#include "core/SkColor.h"
#include "core/SkMatrix.h"
#include "core/SkPaint.h"
#include "core/SkPoint.h"
#include "core/SkPoint3.h"

namespace VGG
{

inline SkRect toSkRect(const VGG::Bound2& bound)
{
  auto rect = SkRect::MakeXYWH(bound.bottomLeft.x,
                               bound.bottomLeft.y,
                               bound.topRight.x - bound.bottomLeft.x,
                               bound.topRight.y - bound.bottomLeft.y);
  return rect;
}

inline SkMatrix toSkMatrix(const glm::mat3& mat)
{
  SkMatrix skMatrix;
  skMatrix.setAll(mat[0][0],
                  mat[1][0],
                  mat[2][0],
                  mat[0][1],
                  mat[1][1],
                  mat[2][1],
                  mat[0][2],
                  mat[1][2],
                  mat[2][2]);
  return skMatrix;
}

inline float calcRotationAngle(const glm::mat3& mat)
{
  glm::vec3 v{ 1, 0, 0 };
  auto r = mat * v;
  const auto v1 = glm::vec2{ r.x, r.y };
  auto d = glm::dot(glm::vec2{ 1, 0 }, v1);
  float cosAngle = d / glm::length(v1);
  float radian = acos(cosAngle);
  return radian * 180.0 / 3.141592653;
}

inline float calcRotationAngle(const SkMatrix& mat)
{
  SkVector r;
  mat.mapVector(1, 0, &r);
  float d = r.dot({ 1, 0 });
  float cosAngle = r.dot({ 1, 0 }) / r.length();
  float radian = acos(cosAngle);
  return radian * 180.0 / 3.141592653;
}

inline SkColor nodeType2Color(ObjectType type)
{
  switch (type)
  {
    case ObjectType::VGG_PATH:
      return SK_ColorRED;
    case ObjectType::VGG_IMAGE:
      return SK_ColorRED;
    case ObjectType::VGG_GROUP:
      return SK_ColorRED;
    case ObjectType::VGG_TEXT:
      return SK_ColorRED;
    case ObjectType::VGG_ARTBOARD:
      return SK_ColorRED;
    case ObjectType::VGG_LAYER:
      return SK_ColorRED;
    case ObjectType::VGG_MASTER:
      return SK_ColorRED;
    case ObjectType::VGG_CONTOUR:
      return SK_ColorYELLOW;
    default:
      return SK_ColorRED;
  }
}
}; // namespace VGG
