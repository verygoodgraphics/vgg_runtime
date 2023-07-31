#include "Core/PathNodePrivate.h"

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

namespace VGG
{

sk_sp<SkShader> PathNode__pImpl::getGradientShader(const Gradient& g, const Bound2& bound)
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

SkPath PathNode__pImpl::makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct)
{
  // return skpath;
  assert(ct.size() >= 1);

  if (ct.size() == 1)
  {
    return ct[0].first;
  }

  std::vector<SkPath> res;
  SkPath skPath = ct[0].first;
  for (int i = 1; i < ct.size(); i++)
  {
    SkPath rhs;
    auto op = ct[i].second;
    if (op != BO_None)
    {
      auto skop = toSkPathOp(op);
      rhs = ct[i].first;
      Op(skPath, rhs, skop, &skPath);
    }
    else
    {
      res.push_back(skPath);
      skPath = ct[i].first;
    }
    op = ct[i].second;
  }
  res.push_back(skPath);

  SkPath paths;
  for (const auto s : res)
  {
    paths.addPath(s);
  }
  return paths;
}

void PathNode__pImpl::drawPathBorder(SkCanvas* canvas,
                                     const SkPath& skPath,
                                     const Border& b,
                                     float globalAlpha,
                                     const Bound2& bound)
{
  SkPaint strokePen;
  strokePen.setAntiAlias(true);
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

SkPaint PathNode__pImpl::makeBlurPen(const Blur& blur)
{
  SkPaint pen;
  pen.setAntiAlias(true);
  auto sigma = SkBlurMask::ConvertRadiusToSigma(blur.radius);
  if (blur.blurType == BT_Gaussian)
  {
    pen.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));
  }
  else if (blur.blurType == BT_Motion)
  {
    pen.setImageFilter(SkImageFilters::Blur(sigma, 0, nullptr));
  }
  return pen;
}

void PathNode__pImpl::drawContour(SkCanvas* canvas,
                                  const ContextSetting& contextSetting,
                                  const Style& style,
                                  const SkPath& skPath,
                                  const Bound2& bound)
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

  // draw fills

  // q_ptr->paintFill(canvas, globalAlpha, style, skPath, bound);
  // draw boarders
  // SkPaint strokePen;
  // strokePen.setAntiAlias(true);
  // strokePen.setStyle(SkPaint::kStroke_Style);
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

void PathNode__pImpl::drawBeforeFill(SkCanvas* canvas,
                                     const Style& style,
                                     const SkPath& skPath,
                                     const Bound2& bound)
{

  const auto filled = hasFill(style);
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

void PathNode__pImpl::drawShadow(SkCanvas* canvas,
                                 const SkPath& skPath,
                                 const Shadow& s,
                                 SkPaint::Style style,
                                 const Bound2& bound)
{

  SkPaint pen;
  pen.setAntiAlias(true);
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

void PathNode__pImpl::drawInnerShadow(SkCanvas* canvas,
                                      const SkPath& skPath,
                                      const Shadow& s,
                                      SkPaint::Style style,
                                      const Bound2& bound)
{
  SkPaint pen;
  auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setImageFilter(
    SkMyImageFilters::DropInnerShadowOnly(s.offset_x, -s.offset_y, sigma, sigma, s.color, nullptr));
  canvas->saveLayer(nullptr, &pen);
  if (s.spread > 0)
    canvas->scale(1.0 / s.spread, 1.0 / s.spread);
  SkPaint fillPen;
  fillPen.setStyle(style);
  fillPen.setAntiAlias(true);
  // if (mask)
  // {
  //   canvas->clipPath(*mask);
  // }
  canvas->drawPath(skPath, fillPen);
  canvas->restore();
}

void PathNode__pImpl::drawFill(SkCanvas* canvas,
                               float globalAlpha,
                               const Style& style,
                               const SkPath& skPath,
                               const Bound2& bound)
{
  for (const auto& f : style.fills)
  {
    if (!f.isEnabled)
      continue;
    SkPaint fillPen;
    fillPen.setStyle(SkPaint::kFill_Style);
    fillPen.setAntiAlias(true);
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
} // namespace VGG
