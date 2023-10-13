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
#include "VSkImageFilters.hpp"
#include "Math/Geometry.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"

#include <core/SkBlendMode.h>
#include <core/SkImageFilter.h>
#include <include/core/SkClipOp.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathTypes.h>
#include <include/core/SkPathEffect.h>
#include <include/effects/SkImageFilters.h>
#include <include/core/SkShader.h>
#include <include/core/SkTypes.h>
#include <include/effects/SkRuntimeEffect.h>
#include <include/core/SkCanvas.h>

#include <src/core/SkBlurMask.h>

#include <algorithm>
using namespace VGG;
class StyleRenderer
{
  bool m_antiAlias{ true };
  SkCanvas* m_canvas{ nullptr };

public:
  StyleRenderer(SkCanvas* canvas, float globalAlpha = 1.0)
    : m_canvas(canvas)
  {
  }
  template<typename F>
  void drawContour(SkCanvas* canvas,
                   const ContextSetting& contextSetting,
                   const Style& style,
                   const SkPath& skPath,
                   const Bound2& bound,
                   sk_sp<SkImageFilter> imageFilter,
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
          drawShadow(canvas, skPath, s, SkPaint::kFill_Style, bound, imageFilter);

        for (const auto& b : style.borders)
        {
          if (!b.isEnabled)
            continue;
          drawShadow(canvas, skPath, s, SkPaint::kStroke_Style, bound, imageFilter);
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
      drawPathBorder(canvas, skPath, b, globalAlpha, bound, imageFilter);
    }

    // draw inner shadow
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kIntersect);

    for (const auto& s : style.shadows)
    {
      if (!s.is_enabled || !s.inner)
        continue;
      drawInnerShadow(canvas, skPath, s, SkPaint::kFill_Style, bound, imageFilter);
    }
    canvas->restore();
  }

  template<typename F>
  void drawStyle(const SkPath& skPath,
                 const Bound2& bound,
                 const ContextSetting& contextSetting,
                 const Style& style,
                 sk_sp<SkImageFilter> imageFilter,
                 F&& drawFill)
  {
    const auto globalAlpha = contextSetting.Opacity;
    const auto filled = hasFill(style);
    // draw outer shadows
    // 1. check out fills
    {
      if (filled)
      {
        // transparent fill clip out the shadow
        m_canvas->save();
        m_canvas->clipPath(skPath, SkClipOp::kDifference);
      }
      for (const auto& s : style.shadows) // simplified into one shadow
      {
        if (!s.is_enabled || s.inner)
          continue;
        if (filled)
          drawShadow(skPath, bound, s, SkPaint::kFill_Style, imageFilter);

        for (const auto& b : style.borders)
        {
          if (!b.isEnabled)
            continue;
          drawShadow(skPath, bound, s, SkPaint::kStroke_Style, imageFilter);
          break;
        }
      }
      if (filled)
      {
        m_canvas->restore();
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
      drawPathBorder(skPath, bound, b, globalAlpha, imageFilter, nullptr);
    }

    // draw inner shadow
    m_canvas->save();
    m_canvas->clipPath(skPath, SkClipOp::kIntersect);

    for (const auto& s : style.shadows)
    {
      if (!s.is_enabled || !s.inner)
        continue;
      drawInnerShadow(skPath, bound, s, SkPaint::kFill_Style, imageFilter);
    }
    m_canvas->restore();
  }

  [[deprecated]] void drawShadow(SkCanvas* canvas,
                                 const SkPath& skPath,
                                 const Shadow& s,
                                 SkPaint::Style style,
                                 const Bound2& bound,
                                 sk_sp<SkImageFilter> imageFilter);

  void drawShadow(const SkPath& skPath,
                  const Bound2& bound,
                  const Shadow& s,
                  SkPaint::Style style,
                  sk_sp<SkImageFilter> imageFilter);

  [[deprecated]] void drawInnerShadow(SkCanvas* canvas,
                                      const SkPath& skPath,
                                      const Shadow& s,
                                      SkPaint::Style style,
                                      const Bound2& bound,
                                      sk_sp<SkImageFilter> imageFilter);

  void drawInnerShadow(const SkPath& skPath,
                       const Bound2& bound,
                       const Shadow& s,
                       SkPaint::Style style,
                       sk_sp<SkImageFilter> imageFilter);

  SkPaint makeBlurPen(const Blur& blur, sk_sp<SkImageFilter> imageFilter);

  bool hasFill(const Style& style) const
  {
    for (const auto& f : style.fills)
    {
      if (f.isEnabled)
        return true;
    }
    return false;
  }

  [[deprecated]] void drawFill(SkCanvas* canvas,
                               float globalAlpha,
                               const Style& style,
                               const SkPath& skPath,
                               const Bound2& bound,
                               sk_sp<SkImageFilter> imageFilter,
                               sk_sp<SkBlender> blender);

  void drawFill(const SkPath& skPath,
                const Bound2& bound,
                const Fill& f,
                float globalAlpha,
                sk_sp<SkImageFilter> imageFilter,
                sk_sp<SkBlender> blender);

  sk_sp<SkShader> getGradientShader(const Gradient& g, const Bound2& bound);

  [[deprecated]] void drawPathBorder(SkCanvas* canvas,
                                     const SkPath& skPath,
                                     const Border& b,
                                     float globalAlpha,
                                     const Bound2& bound,
                                     sk_sp<SkImageFilter> imageFilter);

  void drawPathBorder(const SkPath& skPath,
                      const Bound2& bound,
                      const Border& b,
                      float globalAlpha,
                      sk_sp<SkImageFilter> imageFilter,
                      sk_sp<SkBlender> blender);

  void drawImage(const Bound2 bound,
                 sk_sp<SkShader> imageShader,
                 sk_sp<SkImageFilter> imageFilter,
                 sk_sp<SkBlender> blender)
  {
    SkPaint p;
    p.setShader(std::move(imageShader));
    p.setBlender(std::move(blender));
    p.setImageFilter(std::move(imageFilter));
    m_canvas->drawPaint(p);
  }
};
