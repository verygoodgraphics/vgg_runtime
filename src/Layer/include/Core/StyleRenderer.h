#pragma once
#include "Core/Geometry.hpp"
#include "Core/VType.h"
#include "Core/PathNode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "SkiaImpl/VSkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/gpu/GrTypes.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkBlurMask.h"
#include <algorithm>
#include <core/SkCanvas.h>
class StyleRenderer
{
public:
  SkPath makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct);

  template<typename F>
  void drawContour(SkCanvas* canvas,
                   const ContextSetting& contextSetting,
                   const Style& style,
                   const SkPath& skPath,
                   const Bound2& bound,
                   F&& drawFill)
  {

    // winding rule

    const auto globalAlpha = contextSetting.Opacity;
    const auto filled = hasFill(style);
    // draw outer shadows
    // 1. check out fills
    {
      if (filled)
      {
        // transparent fill clip out the shadow
        canvas->save();
        canvas->clipPath(skPath, SkClipOp::kDifference);
      }
      for (const auto& s : style.shadows) // simplified into one shadow
      {
        if (!s.is_enabled || s.inner)
          continue;
        if (filled)
          drawShadow(canvas, skPath, s, SkPaint::kFill_Style, bound);

        for (const auto& b : style.borders)
        {
          if (!b.isEnabled)
            continue;
          drawShadow(canvas, skPath, s, SkPaint::kStroke_Style, bound);
          break;
        }
      }
      if (filled)
      {
        canvas->restore();
      }
    }

    // draw kFill_Style
    drawFill();
    //  draw boarders
    //  SkPaint strokePen;
    //  strokePen.setAntiAlias(true);
    //  strokePen.setStyle(SkPaint::kStroke_Style);
    for (const auto& b : style.borders)
    {
      if (!b.isEnabled)
        continue;
      drawPathBorder(canvas, skPath, b, globalAlpha, bound);
    }

    // draw inner shadow
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kIntersect);
    for (const auto& s : style.shadows)
    {
      if (!s.is_enabled || !s.inner)
        continue;
      drawInnerShadow(canvas, skPath, s, SkPaint::kFill_Style, bound);
    }
    canvas->restore();
  }

  void drawShadow(SkCanvas* canvas,
                  const SkPath& skPath,
                  const Shadow& s,
                  SkPaint::Style style,
                  const Bound2& bound);

  void drawInnerShadow(SkCanvas* canvas,
                       const SkPath& skPath,
                       const Shadow& s,
                       SkPaint::Style style,
                       const Bound2& bound);

  SkPaint makeBlurPen(const Blur& blur);

  bool hasFill(const Style& style) const
  {
    for (const auto& f : style.fills)
    {
      if (f.isEnabled)
        return true;
    }
    return false;
  }

  void drawFill(SkCanvas* canvas,
                float globalAlpha,
                const Style& style,
                const SkPath& skPath,
                const Bound2& bound);

  sk_sp<SkShader> getGradientShader(const Gradient& g, const Bound2& bound);

  void drawPathBorder(SkCanvas* canvas,
                      const SkPath& skPath,
                      const Border& b,
                      float globalAlpha,
                      const Bound2& bound);
};
