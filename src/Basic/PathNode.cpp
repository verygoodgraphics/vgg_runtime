#include "Basic/Attrs.h"
#include "Basic/ContourNode.h"
#include "Basic/PaintNode.h"
#include "Basic/PathNode.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "Basic/SkiaBackend/SkiaConverter.h"
#include "Basic/SkiaBackend/SkiaUtils.h"

#include "Components/Styles.hpp"
#include "Utils/Utils.hpp"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"

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
#include <limits>

namespace VGG
{

// void drawPathFill(SkCanvas* canvas,
//                   const Frame& frame,
//                   const SkPath& skPath,
//                   const FillStyle* style,
//                   double globalAlpha)
// {
//   ASSERT(canvas);
//   SkPaint fillPen;
//   fillPen.setStyle(SkPaint::kFill_Style);
//
//   if (!style)
//   {
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isFlat(*style))
//   {
//     fillPen.setColor(style->color);
//     fillPen.setAlphaf(fillPen.getAlphaf() * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isLinearGradient(*style))
//   {
//     fillPen.setShader(style->gradient.getLinearShader(frame));
//     fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isRadialGradient(*style))
//   {
//     fillPen.setShader(style->gradient.getRadialShader(frame));
//     fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (FillTypeChooser<FillStyle>::isAngularGradient(*style))
//   {
//     fillPen.setShader(style->gradient.getAngularShader(frame));
//     fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//     canvas->drawPath(skPath, fillPen);
//   }
//   else if (style->fillType == FillStyle::FillType::IMAGE)
//   {
//     if (auto& name = style->imageName)
//     {
//       fillPen.setShader(style->getImageShader(frame));
//       fillPen.setAlphaf(style->contextSettings.opacity * globalAlpha);
//       canvas->drawPath(skPath, fillPen);
//     }
//   }
// }

// bool maskedByOthers = !maskedBy.empty();
// auto cvs = canvas;
// sk_sp<SkSurface> objectTarget;
// if (maskedByOthers)
// {
//   objectTarget = SkSurface::MakeRenderTarget(canvas->recordingContext(),
//                                              SkBudgeted::kYes,
//                                              canvas->imageInfo(),
//                                              0,
//                                              GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
//                                              &canvas->getSurface()->props(),
//                                              false);
//   if (objectTarget)
//   {
//     cvs = objectTarget->getCanvas();
//   }
//   else
//   {
//     WARN("target cannot be created\n");
//   }
// }
//
// SkPaint p;
// p.setShader(shader);
// cvs->drawRect(toSkRect(this->bound), p);

// if (maskedByOthers)
// {
//   auto target = SkSurface::MakeRenderTarget(canvas->recordingContext(),
//                                             SkBudgeted::kYes,
//                                             canvas->imageInfo(),
//                                             0,
//                                             GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
//                                             &canvas->getSurface()->props(),
//                                             false);
//
//   SkPaint fill;
//   fill.setColor4f({ 1, 0, 0, 1 });
//   fill.setStyle(SkPaint::kFill_Style);
//   auto c = target->getCanvas();
//   c->concat(canvas->getTotalMatrix());
//   c->drawRect(toSkRect(this->bound), fill);
//
//   // using shader
//   if (objectTarget)
//   {
//     SkSamplingOptions opt;
//     sk_sp<SkShader> objectShader =
//       objectTarget->makeImageSnapshot()->makeShader(SkTileMode::kClamp,
//                                                     SkTileMode::kClamp,
//                                                     opt,
//                                                     nullptr);
//     auto maskShader = target->makeImageSnapshot()->makeShader(SkTileMode::kClamp,
//                                                               SkTileMode::kClamp,
//                                                               opt,
//                                                               nullptr);
//
//     const char* sksl = "uniform shader input_1;"
//                        "uniform shader input_2;"
//                        "uniform vec4 rect;"
//                        "half4 main(float2 coord) {"
//                        "  return sample(input_2, coord);"
//                        "}";
//
//     // Create SkShader from SkSL, then fill surface: // SK_FOLD_START
//
//     // Create an SkShader from our SkSL, with `children` bound to the inputs:
//     auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(sksl));
//     if (effect)
//     {
//       maskEffect = effect;
//
//       const auto info = canvas->imageInfo();
//       SkRuntimeShaderBuilder builder(effect);
//       builder.uniform("rect") = SkV4{ (float)info.width(), (float)info.height(), 0, 0 };
//       builder.child("input_1") = objectShader;
//       builder.child("input_2") = maskShader;
//       SkPaint maskPaint;
//       auto shader = builder.makeShader(nullptr, false);
//       maskPaint.setShader(shader);
//       // canvas->drawPaint(maskPaint);
//     }
//     else
//     {
//       WARN("%s", err.c_str());
//     }
//   }
// }
//

void drawPathFill(SkCanvas* canvas,
                  const glm::vec2& size,
                  const SkPath& path,
                  const Style& fillStyle,
                  float globalAlpha)
{
}

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

PathNode::PathNode(const std::string& name)
  : PaintNode(name, ObjectType::VGG_PATH)
{
}

Mask PathNode::asOutlineMask(const glm::mat3* mat)
{
  Mask mask;
  for (const auto& c : m_firstChild)
  {
    auto p = static_cast<PaintNode*>(c.get());
    mask.addMask(p->asOutlineMask(&p->transform));
  }
  if (mat)
  {
    mask.outlineMask.transform(toSkMatrix(*mat));
  }
  return mask;
}

void PathNode::paintEvent(SkCanvas* canvas)
{
  if (m_firstChild.empty())
    return;
  auto mask = makeMaskBy(BO_Intersection);
  if (mask.outlineMask.isEmpty())
  {
    drawContour(canvas, nullptr);
  }
  else
  {
    drawContour(canvas, &mask.outlineMask);
  }
} // namespace VGG

SkPath PathNode::makePath()
{
  std::vector<std::pair<SkPath, EBoolOp>> ct;
  // SkPath skpath;
  for (const auto& c : m_firstChild)
  {
    auto p = static_cast<PaintNode*>(c.get());
    auto outline = p->asOutlineMask(&p->transform);
    ct.emplace_back(outline.outlineMask, p->clipOperator());
  }
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

void PathNode::drawContour(SkCanvas* canvas, const SkPath* outlineMask)
{

  SkPath skPath = makePath();
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
    if (f.fillType == FT_Color)
    {
      fillPen.setColor(f.color);
      fillPen.setAlphaf(fillPen.getAlphaf() * globalAlpha);
    }
    else if (f.fillType == FT_Gradient)
    {
      fillPen.setShader(getGradientShader(f.gradient.value(), bound.size()));
      fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
    }
    fillPen.setStyle(SkPaint::kFill_Style);
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

    if (b.gradient.has_value() && b.fill_type == FT_Gradient)
    {
      strokePen.setShader(getGradientShader(b.gradient.value(), bound.size()));
      strokePen.setAlphaf(b.context_settings.Opacity * globalAlpha);
    }
    else
    {
      strokePen.setColor(b.color.value_or(VGGColor{ .r = 0, .g = 0, .b = 0, .a = 1.0 }));
      strokePen.setAlphaf(strokePen.getAlphaf() * globalAlpha);
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

void PathNode::addSubShape(std::shared_ptr<PaintNode> node, EBoolOp op)
{
  node->setClipOperator(op);
  addChild(node);
}

} // namespace VGG
