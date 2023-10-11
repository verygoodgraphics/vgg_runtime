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
#include "StyleRenderer.hpp"
#include "Layer/Core/VType.hpp"
#include "VSkia.hpp"

using namespace VGG;

sk_sp<SkShader> StyleRenderer::getGradientShader(const Gradient& g, const Bound2& bound)
{
  sk_sp<SkShader> shader;
  const auto type = g.gradientType;
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

void StyleRenderer::drawPathBorder(SkCanvas* canvas,
                                   const SkPath& skPath,
                                   const Border& b,
                                   float globalAlpha,
                                   const Bound2& bound,
                                   sk_sp<SkImageFilter> imageFilter)
{
  SkPaint strokePen;
  strokePen.setAntiAlias(m_antiAlias);
  strokePen.setBlendMode(m_mode);
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
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kIntersect);
  }
  else if (b.position == PP_Outside)
  {
    // outside
    strokePen.setStrokeWidth(2. * b.thickness);
    canvas->save();
    canvas->clipPath(skPath, SkClipOp::kDifference);
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
    auto img = loadImage(b.pattern->imageGUID, Scene::getResRepo());
    if (!img)
      return;
    auto bs = bound.size();
    const auto m = toSkMatrix(b.pattern->transform);
    auto shader = getImageShader(img,
                                 bs.x,
                                 bs.y,
                                 b.pattern->imageFillType,
                                 b.pattern->tileScale,
                                 b.pattern->tileMirrored,
                                 &m);
    strokePen.setShader(shader);
    strokePen.setAlphaf(b.context_settings.Opacity * globalAlpha);
  }

  canvas->drawPath(skPath, strokePen);

  if (b.position != PP_Center)
  {
    canvas->restore(); // pop border position style
  }
}

SkPaint StyleRenderer::makeBlurPen(const Blur& blur, sk_sp<SkImageFilter> imageFilter)
{
  SkPaint pen;
  pen.setAntiAlias(m_antiAlias);
  auto sigma = SkBlurMask::ConvertRadiusToSigma(blur.radius);
  if (blur.blurType == BT_Gaussian)
  {
    pen.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));
  }
  else if (blur.blurType == BT_Motion)
  {
    pen.setImageFilter(SkImageFilters::Blur(sigma, 0, nullptr));
  }
  else if (blur.blurType == VGG::BT_Background)
  {
    sk_sp<SkImageFilter> newBlurFilter = SkImageFilters::Blur(5, 5, SkTileMode::kDecal, nullptr);
    pen.setImageFilter(std::move(newBlurFilter));
  }
  return pen;
}

void StyleRenderer::drawShadow(SkCanvas* canvas,
                               const SkPath& skPath,
                               const Shadow& s,
                               SkPaint::Style style,
                               const Bound2& bound,
                               sk_sp<SkImageFilter> imageFilter)
{

  SkPaint pen;
  pen.setAntiAlias(m_antiAlias);
  pen.setBlendMode(m_mode);
  auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setImageFilter(
    SkImageFilters::DropShadowOnly(s.offset_x, -s.offset_y, sigma, sigma, s.color, nullptr));
  canvas->saveLayer(nullptr, &pen);
  if (s.spread > 0)
    canvas->scale(1 + s.spread / 100.0, 1 + s.spread / 100.0);
  SkPaint fillPen;
  fillPen.setStyle(style);
  fillPen.setAntiAlias(true);
  // if (outlineMask)
  // {
  //   canvas->clipPath(*outlineMask);
  // }
  canvas->drawPath(skPath, fillPen);
  canvas->restore();
}

void StyleRenderer::drawInnerShadow(SkCanvas* canvas,
                                    const SkPath& skPath,
                                    const Shadow& s,
                                    SkPaint::Style style,
                                    const Bound2& bound,
                                    sk_sp<SkImageFilter> imageFilter)
{
  SkPaint pen;
  auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setAntiAlias(m_antiAlias);
  pen.setImageFilter(
    SkMyImageFilters::DropInnerShadowOnly(s.offset_x, -s.offset_y, sigma, sigma, s.color, nullptr));
  canvas->saveLayer(nullptr, &pen);
  if (s.spread > 0)
    canvas->scale(1.0 / s.spread, 1.0 / s.spread);
  SkPaint fillPen;
  fillPen.setStyle(style);
  fillPen.setAntiAlias(m_antiAlias);
  // if (mask)
  // {
  //   canvas->clipPath(*mask);
  // }
  canvas->drawPath(skPath, fillPen);
  canvas->restore();
}

void StyleRenderer::drawFill(SkCanvas* canvas,
                             float globalAlpha,
                             const Style& style,
                             const SkPath& skPath,
                             const Bound2& bound,
                             sk_sp<SkImageFilter> imageFilter)
{
  for (const auto& f : style.fills)
  {
    if (!f.isEnabled)
      continue;
    SkPaint fillPen;
    fillPen.setStyle(SkPaint::kFill_Style);
    fillPen.setAntiAlias(m_antiAlias);
    fillPen.setBlendMode(m_mode);
    fillPen.setImageFilter(imageFilter);
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
      auto img = loadImage(f.pattern->imageGUID, Scene::getResRepo());
      if (!img)
        continue;
      auto bs = bound.size();
      const auto m = toSkMatrix(f.pattern->transform);
      auto shader = getImageShader(img,
                                   bs.x,
                                   bs.y,
                                   f.pattern->imageFillType,
                                   f.pattern->tileScale,
                                   f.pattern->tileMirrored,
                                   &m);

      fillPen.setShader(shader);
      fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
    }
    canvas->drawPath(skPath, fillPen);
    // if (f.fillType == FT_Gradient && f.gradient)
    // {
    //   SkPaint debugLine;
    //   debugLine.setColor(SK_ColorBLUE);
    //   debugLine.setStrokeWidth(2);
    //   const auto bound = getBound();
    //   auto from = bound.map(bound.size() * f.gradient->from);
    //   auto to = bound.map(bound.size() * f.gradient->to);
    //   if (f.gradient->aiCoordinate)
    //   {
    //     auto r = f.gradient->aiConvert(f.gradient->from, f.gradient->to, bound);
    //     from = r.first;
    //     to = r.second;
    //   }
    //   canvas->save();
    //   canvas->drawLine(from.x, from.y, to.x, to.y, debugLine);
    //   SkPaint debugPoint;
    //   debugPoint.setColor(SK_ColorRED);
    //   debugPoint.setStrokeWidth(2);
    //   canvas->drawPoint(from.x, from.y, debugPoint);
    //   canvas->restore();
    // }
  }
}
