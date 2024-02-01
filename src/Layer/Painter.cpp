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
#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <core/SkMaskFilter.h>
#include <core/SkPaint.h>
#include <core/SkTextBlob.h>
#include <string>

using namespace VGG;

using namespace VGG::layer;

sk_sp<SkShader> getGradientShader(const Gradient& g, const Bound& bound)
{
  return makeGradientShader(bound, g);
}

void Painter::drawPathBorder(
  const SkPath&        skPath,
  const Bound&         bound,
  const Border&        b,
  sk_sp<SkImageFilter> imageFilter,
  sk_sp<SkBlender>     blender)
{
  SkPaint strokePen;
  strokePen.setAntiAlias(m_antiAlias);
  strokePen.setBlender(blender);
  strokePen.setImageFilter(imageFilter);
  populateSkPaint(b, toSkRect(bound), strokePen);
  bool  inCenter = true;
  float strokeWidth = b.thickness;
  if (b.position == PP_Inside && skPath.isLastContourClosed())
  {
    // inside
    strokeWidth = 2.f * b.thickness;
    m_renderer->canvas()->save();
    m_renderer->canvas()->clipPath(skPath, SkClipOp::kIntersect);
    inCenter = false;
  }
  else if (b.position == PP_Outside && skPath.isLastContourClosed())
  {
    // outside
    strokeWidth = 2.f * b.thickness;
    m_renderer->canvas()->save();
    m_renderer->canvas()->clipPath(skPath, SkClipOp::kDifference);
    inCenter = false;
  }
  strokePen.setStrokeWidth(strokeWidth);
  m_renderer->canvas()->drawPath(skPath, strokePen);
  if (!inCenter)
  {
    m_renderer->canvas()->restore();
  }

  if (false)
  {
    std::vector<SkPoint> pts(skPath.countPoints());
    SkPaint              p;
    skPath.getPoints(pts.data(), skPath.countPoints());
    p.setStrokeWidth(2);
    p.setColor(SK_ColorRED);
    SkFont a;
    m_renderer->canvas()->drawPoints(SkCanvas::kPoints_PointMode, pts.size(), pts.data(), p);
    for (std::size_t i = 0; i < pts.size(); i++)
    {
      SkPaint textPaint;
      textPaint.setStrokeWidth(0.5);
      textPaint.setColor(SK_ColorBLACK);
      std::string index = std::to_string(i);
      m_renderer->canvas()->drawSimpleText(
        index.c_str(),
        index.size(),
        SkTextEncoding::kUTF8,
        pts[i].x(),
        pts[i].y(),
        a,
        textPaint);
    }
  }
}

void Painter::drawShadow(
  const SkPath&        skPath,
  const Bound&         bound,
  const Shadow&        s,
  SkPaint::Style       style,
  sk_sp<SkImageFilter> imageFilter)
{
  SkPaint pen;
  pen.setAntiAlias(m_antiAlias);
  auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setImageFilter(
    SkImageFilters::DropShadowOnly(s.offsetX, s.offsetY, sigma, sigma, s.color, nullptr));
  m_renderer->canvas()->saveLayer(nullptr, &pen); // TODO:: test hint rect
  if (s.spread > 0)
    m_renderer->canvas()->scale(1 + s.spread / 100.0, 1 + s.spread / 100.0);
  SkPaint fillPen;
  fillPen.setStyle(style);
  m_renderer->canvas()->drawPath(skPath, fillPen);
  m_renderer->canvas()->restore();
}

void Painter::drawInnerShadow(
  const SkPath&        skPath,
  const Bound&         bound,
  const Shadow&        s,
  SkPaint::Style       style,
  sk_sp<SkImageFilter> imageFilter)
{
  SkPaint pen;
  auto    sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setAntiAlias(m_antiAlias);
  pen.setImageFilter(
    SkMyImageFilters::DropInnerShadowOnly(s.offsetX, s.offsetY, sigma, sigma, s.color, nullptr));
  m_renderer->canvas()->saveLayer(nullptr, &pen);
  if (s.spread > 0)
    m_renderer->canvas()->scale(1.0 / s.spread, 1.0 / s.spread);
  SkPaint fillPen;
  fillPen.setStyle(style);
  fillPen.setAntiAlias(m_antiAlias);
  m_renderer->canvas()->drawPath(skPath, fillPen);
  m_renderer->canvas()->restore();
}

void Painter::drawFill(
  const SkPath&        skPath,
  const Bound&         bound,
  const Fill&          f,
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
  populateSkPaint(f.type, f.contextSettings, toSkRect(bound), fillPen);
  m_renderer->canvas()->drawPath(skPath, fillPen);
}
