#include "PathNodePrivate.h"

#include "Basic/VGGType.h"
#include "include/effects/SkImageFilters.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/gpu/GrTypes.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkBlurMask.h"

sk_sp<SkShader> getGradientShader(const VGGGradient& g, const glm::vec2& size)
{
  sk_sp<SkShader> shader;
  const auto type = g.gradientType;
  if (type == GT_Linear)
  {
    shader = g.getLinearShader(size);
  }
  else if (type == GT_Radial)
  {
    shader = g.getRadialShader(size);
  }
  else if (type == GT_Angular)
  {
    shader = g.getAngularShader(size);
  }
  return shader;
}

SkPath makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct)
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

void drawContour(SkCanvas* canvas,
                 const SkPath* outlineMask,
                 const ContextSetting& contextSetting,
                 const Style& style,
                 EWindingType windingRule,
                 const std::vector<std::pair<SkPath, EBoolOp>>& ct,
                 const Bound2& bound)
{

  SkPath skPath = makePath(ct);
  // winding rule
  if (windingRule == EWindingType::WR_EvenOdd)
  {
    skPath.setFillType(SkPathFillType::kEvenOdd);
  }
  else
  {
    skPath.setFillType(SkPathFillType::kWinding);
  }

  const auto globalAlpha = contextSetting.Opacity;

  // draw outer shadows
  for (const auto& s : style.shadows)
  {
    if (!s.is_enabled || s.inner)
      continue;
    drawShadow(canvas, skPath, s, outlineMask, false);
  }

  // draw fills
  for (const auto& f : style.fills)
  {
    if (!f.isEnabled)
      continue;
    SkPaint fillPen;
    fillPen.setStyle(SkPaint::kFill_Style);
    if (f.fillType == FT_Color)
    {
      fillPen.setColor(f.color);
      fillPen.setAlphaf(fillPen.getAlphaf() * globalAlpha);
    }
    else if (f.fillType == FT_Gradient)
    {
      assert(f.gradient.has_value());
      auto gradientShader = getGradientShader(f.gradient.value(), bound.size());
      fillPen.setShader(gradientShader);
      fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
    }
    else if (f.fillType == FT_Pattern)
    {
      assert(f.pattern.has_value());
      auto img = loadImage(f.pattern->imageGUID, Scene::getResRepo());
      auto bs = bound.size();
      auto shader = getImageShader(img,
                                   bs.x,
                                   bs.y,
                                   f.pattern->imageFillType,
                                   f.pattern->tileScale,
                                   f.pattern->tileMirrored);

      fillPen.setShader(shader);
      fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
    }
    if (outlineMask)
    {
      canvas->clipPath(*outlineMask);
    }
    canvas->drawPath(skPath, fillPen);
  }

  // draw boarders
  SkPaint strokePen;
  strokePen.setAntiAlias(true);
  strokePen.setStyle(SkPaint::kStroke_Style);
  for (const auto& b : style.borders)
  {
    if (!b.is_enabled)
      continue;
    strokePen.setAntiAlias(true);
    strokePen.setStrokeJoin(toSkPaintJoin(b.line_join_style));
    strokePen.setStrokeCap(toSkPaintCap(b.line_cap_style));
    strokePen.setColor(b.color.value_or(VGGColor{ .r = 0, .g = 0, .b = 0, .a = 1.0 }));
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
      strokePen.setShader(getGradientShader(b.gradient.value(), bound.size()));
      strokePen.setAlphaf(b.context_settings.Opacity * globalAlpha);
    }
    else if (b.fill_type == FT_Color)
    {
      strokePen.setColor(b.color.value_or(VGGColor{ .r = 0, .g = 0, .b = 0, .a = 1.0 }));
      strokePen.setAlphaf(strokePen.getAlphaf() * globalAlpha);
    }
    else if (b.fill_type == FT_Pattern)
    {

      assert(b.pattern.has_value());
      auto img = loadImage(b.pattern->imageGUID, Scene::getResRepo());
      auto bs = bound.size();
      auto shader = getImageShader(img,
                                   bs.x,
                                   bs.y,
                                   b.pattern->imageFillType,
                                   b.pattern->tileScale,
                                   b.pattern->tileMirrored);

      strokePen.setShader(shader);
      strokePen.setAlphaf(b.context_settings.Opacity * globalAlpha);
    }

    if (outlineMask)
    {
      canvas->clipPath(*outlineMask);
    }
    canvas->drawPath(skPath, strokePen);

    if (b.position != PP_Center)
    {
      canvas->restore(); // pop border position style
    }
  }

  // draw inner shadow
  for (const auto& s : style.shadows)
  {
    if (!s.is_enabled || !s.inner)
      continue;
    drawShadow(canvas, skPath, s, outlineMask, true);
  }
}

void drawShadow(SkCanvas* canvas,
                const SkPath& skPath,
                const Shadow& s,
                const SkPath* outlineMask,
                bool inner)
{

  SkPaint pen;
  auto sigma = SkBlurMask::ConvertRadiusToSigma(s.blur);
  pen.setImageFilter(
    SkImageFilters::DropShadowOnly(s.offset_x, s.offset_y, sigma, sigma, s.color, nullptr));
  canvas->saveLayer(nullptr, &pen);
  canvas->translate(s.offset_x, s.offset_y);
  if (inner)
  {
    canvas->scale(s.spread, s.spread);
  }
  else
  {
    if (s.spread > 0)
    {
      canvas->scale(1 / s.spread, 1 / s.spread);
    }
  }
  canvas->translate(-s.offset_x, -s.offset_y);
  SkPaint fillPen;
  fillPen.setStyle(SkPaint::kFill_Style);

  if (outlineMask)
  {
    canvas->clipPath(*outlineMask);
  }

  canvas->drawPath(skPath, fillPen);
  canvas->restore();
}
