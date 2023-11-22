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
#include "Painter.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/PathGenerator.hpp"
#include "VSkia.hpp"
#include <core/SkColor.h>
#include <core/SkMaskFilter.h>
#include <core/SkPaint.h>

using namespace VGG;

sk_sp<SkShader> getGradientShader(const Gradient& g, const Bound& bound)
{
  sk_sp<SkShader> shader;
  const auto      type = g.gradientType;
  if (type == GT_Linear)
  {
    shader = g.getLinearShader(bound);
  }
  else if (type == GT_Radial)
  {
    shader = g.getRadialShader(bound);
  }
  else if (type == GT_Angular)
  {
    shader = g.getAngularShader(bound);
  }
  return shader;
}

void Painter::drawPathBorder(const SkPath&        skPath,
                             const Bound&         bound,
                             const Border&        b,
                             float                globalAlpha,
                             sk_sp<SkImageFilter> imageFilter,
                             sk_sp<SkBlender>     blender)
{
  SkPaint strokePen;
  strokePen.setAntiAlias(m_antiAlias);
  strokePen.setBlender(blender);
  strokePen.setImageFilter(imageFilter);
  strokePen.setStyle(SkPaint::kStroke_Style);
  strokePen.setPathEffect(
    SkDashPathEffect::Make(b.dashed_pattern.data(), b.dashed_pattern.size(), 0));
  strokePen.setStrokeJoin(toSkPaintJoin(b.lineJoinStyle));
  strokePen.setStrokeCap(toSkPaintCap(b.lineCapStyle));
  strokePen.setStrokeMiter(b.miterLimit);
  strokePen.setColor(b.color.value_or(Color{ .r = 0, .g = 0, .b = 0, .a = 1.0 }));
  if (b.position == PP_Inside)
  {
    // inside
    strokePen.setStrokeWidth(2. * b.thickness);
    m_canvas->save();
    m_canvas->clipPath(skPath, SkClipOp::kIntersect);
  }
  else if (b.position == PP_Outside)
  {
    // outside
    strokePen.setStrokeWidth(2. * b.thickness);
    m_canvas->save();
    m_canvas->clipPath(skPath, SkClipOp::kDifference);
  }
  else
  {
    strokePen.setStrokeWidth(b.thickness);
  }

  // draw fill for border
  if (b.fill_type == FT_Gradient)
  {
    assert(b.gradient.has_value());
    strokePen.setShader(getGradientShader(b.gradient.value(), bound));
    strokePen.setAlphaf(b.context_settings.Opacity * globalAlpha);
  }
  else if (b.fill_type == FT_Color)
  {
    strokePen.setColor(b.color.value_or(Color{ .r = 0, .g = 0, .b = 0, .a = 1.0 }));
    strokePen.setAlphaf(strokePen.getAlphaf() * globalAlpha);
  }
  else if (b.fill_type == FT_Pattern)
  {
    assert(b.pattern.has_value());
    auto shader = makePatternShader(bound, b.pattern.value());
    strokePen.setShader(shader);
    strokePen.setAlphaf(b.context_settings.Opacity * globalAlpha);
  }

  m_canvas->drawPath(skPath, strokePen);

  if (b.position != PP_Center)
  {
    m_canvas->restore(); // pop border position style
  }
}

void Painter::drawShadow(const SkPath&        skPath,
                         const Bound&         bound,
                         const Shadow&        s,
                         SkPaint::Style       style,
                         sk_sp<SkImageFilter> imageFilter)
{
  SkPaint pen;
  pen.setAntiAlias(m_antiAlias);
  auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setImageFilter(
    SkImageFilters::DropShadowOnly(s.offset_x, -s.offset_y, sigma, sigma, s.color, nullptr));
  m_canvas->saveLayer(nullptr, &pen); // TODO:: test hint rect
  if (s.spread > 0)
    m_canvas->scale(1 + s.spread / 100.0, 1 + s.spread / 100.0);
  SkPaint fillPen;
  fillPen.setStyle(style);
  m_canvas->drawPath(skPath, fillPen);
  m_canvas->restore();
}

void Painter::drawInnerShadow(const SkPath&        skPath,
                              const Bound&         bound,
                              const Shadow&        s,
                              SkPaint::Style       style,
                              sk_sp<SkImageFilter> imageFilter)
{
  SkPaint pen;
  auto    sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setAntiAlias(m_antiAlias);
  pen.setImageFilter(
    SkMyImageFilters::DropInnerShadowOnly(s.offset_x, -s.offset_y, sigma, sigma, s.color, nullptr));
  m_canvas->saveLayer(nullptr, &pen);
  if (s.spread > 0)
    m_canvas->scale(1.0 / s.spread, 1.0 / s.spread);
  SkPaint fillPen;
  fillPen.setStyle(style);
  fillPen.setAntiAlias(m_antiAlias);
  m_canvas->drawPath(skPath, fillPen);
  m_canvas->restore();
}

void Painter::drawFill(const SkPath&        skPath,
                       const Bound&         bound,
                       const Fill&          f,
                       float                globalAlpha,
                       sk_sp<SkImageFilter> imageFilter,
                       sk_sp<SkBlender>     blender,
                       sk_sp<SkMaskFilter>  mask)
{
  SkPaint fillPen;
  fillPen.setStyle(SkPaint::kFill_Style);
  fillPen.setAntiAlias(m_antiAlias);
  fillPen.setBlender(blender);
  fillPen.setImageFilter(imageFilter);
  fillPen.setMaskFilter(mask);
  if (f.fillType == FT_Color)
  {
    fillPen.setColor(f.color);
    const auto currentAlpha = fillPen.getAlphaf();
    fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha * currentAlpha);
  }
  else if (f.fillType == FT_Gradient)
  {
    assert(f.gradient.has_value());
    auto gradientShader = getGradientShader(f.gradient.value(), bound);
    fillPen.setShader(gradientShader);
    fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
  }
  else if (f.fillType == FT_Pattern)
  {
    assert(f.pattern.has_value());
    auto shader = makePatternShader(bound, f.pattern.value());
    fillPen.setShader(shader);
    fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
  }
  m_canvas->drawPath(skPath, fillPen);
}
