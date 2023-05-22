#pragma once
#include "include/core/SkPaint.h"
#include "Basic/VGGType.h"
#include "include/pathops/SkPathOps.h"

inline SkPaint::Join toSkPaintJoin(VGG::ELineJoin join)
{
  switch (join)
  {
    case VGG::LJ_Miter:
      return SkPaint::kMiter_Join;
    case VGG::LJ_Round:
      return SkPaint::kRound_Join;
    case VGG::LJ_Bevel:
      return SkPaint::kBevel_Join;
  }
  return SkPaint::kMiter_Join;
}

inline SkPaint::Cap toSkPaintCap(VGG::ELineCap cap)
{
  switch (cap)
  {
    case VGG::LC_Butt:
      return SkPaint::kButt_Cap;
    case VGG::LC_Round:
      return SkPaint::kRound_Cap;
    case VGG::LC_Square:
      return SkPaint::kSquare_Cap;
  }
  return SkPaint::kButt_Cap;
}

inline SkPathOp toSkPathOp(VGG::EBoolOp blop)
{
  SkPathOp op;
  switch (blop)
  {
    case VGG::BO_Union:
      op = SkPathOp::kUnion_SkPathOp;
      break;
    case VGG::BO_Substraction:
      op = SkPathOp::kDifference_SkPathOp;
      break;
    case VGG::BO_Intersection:
      op = SkPathOp::kIntersect_SkPathOp;
      break;
    case VGG::BO_Exclusion:
      op = SkPathOp::kReverseDifference_SkPathOp;
      break;
    default:
      return SkPathOp::kUnion_SkPathOp;
  }
  return op;
}
