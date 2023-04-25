#pragma once

#include "VGGType.h"
#include "Geometry.hpp"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"

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
                  mat[0][1],
                  mat[0][2],
                  mat[1][0],
                  mat[1][1],
                  mat[1][2],
                  mat[2][0],
                  mat[2][1],
                  mat[2][2]);
  return skMatrix;
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
