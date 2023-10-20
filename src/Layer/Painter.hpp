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
#include "VSkia.hpp"
#include "Layer/Core/Attrs.hpp"

#include <core/SkBlendMode.h>
#include <core/SkImageFilter.h>
#include <core/SkMaskFilter.h>
#include <include/core/SkClipOp.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathTypes.h>
#include <include/core/SkPathEffect.h>
#include <include/effects/SkImageFilters.h>
#include <include/core/SkShader.h>
#include <include/core/SkTypes.h>
#include <include/effects/SkRuntimeEffect.h>
#include <include/core/SkCanvas.h>
#include <pathops/SkPathOps.h>
#include <src/core/SkBlurMask.h>

#include <stack>

using namespace VGG;
class Painter
{
  bool m_antiAlias{ true };
  SkCanvas* m_canvas{ nullptr };

public:
  Painter(SkCanvas* canvas)
    : m_canvas(canvas)
  {
  }

  SkCanvas* canvas()
  {
    return m_canvas;
  }

  void blurBackgroundBegin(float radiusX, float radiusY, const Bound2& bound, const SkPath* path)
  {
    auto filter = SkImageFilters::Blur(SkBlurMask::ConvertRadiusToSigma(radiusX),
                                       SkBlurMask::ConvertRadiusToSigma(radiusY),
                                       nullptr);
    auto b = toSkRect(bound);
    m_canvas->save();
    if (path)
      m_canvas->clipPath(*path);
    m_canvas->saveLayer(SkCanvas::SaveLayerRec(&b, nullptr, filter.get(), 0));
  }

  void blurBackgroundEnd()
  {
    m_canvas->restore();
    m_canvas->restore();
  }
  void blurContentBegin(float radiusX, float radiusY, const Bound2& bound, const SkPath* path)
  {
    SkPaint pen;
    auto filter = SkImageFilters::Blur(SkBlurMask::ConvertRadiusToSigma(radiusX),
                                       SkBlurMask::ConvertRadiusToSigma(radiusY),
                                       nullptr);
    // pen.setImageFilter(std::move(filter));
    pen.setBlendMode(SkBlendMode::kSrcOver);
    auto b = toSkRect(bound);
    SkMatrix m;
    m.preScale(1, 1);
    b = m.mapRect(b);
    m_canvas->save();
    if (path)
      m_canvas->clipPath(*path);
    m_canvas->saveLayer(&b, &pen);
  }

  void blurContentEnd()
  {
    m_canvas->restore();
    m_canvas->restore();
  }

  void beginClip(const SkPath& path, SkClipOp clipOp = SkClipOp::kIntersect)
  {
    m_canvas->save();
    m_canvas->clipPath(path, clipOp);
  }

  void endClip()
  {
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
                sk_sp<SkBlender> blender,
                sk_sp<SkMaskFilter> mask);

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
