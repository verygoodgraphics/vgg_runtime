#pragma once
#include "include/core/SkPaint.h"
#include "Basic/VGGType.h"

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
}
