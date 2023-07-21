#include "Core/Attrs.h"
#include "Core/ContourNode.h"
#include "Core/PaintNode.h"
#include "Core/PathNode.h"
#include "Core/VType.h"
#include "Core/PathNodePrivate.h"
#include "SkiaBackend/SkiaImpl.h"

#include "Log.h"
#include "core/SkBlendMode.h"
#include "core/SkCanvas.h"
#include "core/SkMatrix.h"
#include "core/SkPaint.h"
#include "core/SkPathEffect.h"

#include "effects/SkImageFilters.h"
#include "core/SkScalar.h"
#include "core/SkShader.h"
#include "core/SkSurface.h"
#include "core/SkTileMode.h"
#include "core/SkTypes.h"
#include "effects/SkRuntimeEffect.h"
#include "core/SkColor.h"
#include "core/SkPath.h"
#include "gpu/GrTypes.h"
#include "pathops/SkPathOps.h"
#include "src/core/SkBlurMask.h" // TODO:: remove 'src' prefix after skia upgrade
#include <limits>

namespace VGG
{

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

PathNode::PathNode(const std::string& name)
  : PaintNode(name, ObjectType::VGG_PATH)
  , d_ptr(new PathNode__pImpl(this))
{
}

Mask PathNode::asOutlineMask(const glm::mat3* mat)
{
  Mask mask;
  for (const auto& c : m_firstChild)
  {
    auto p = static_cast<PaintNode*>(c.get());
    mask.addMask(p->asOutlineMask(&p->localTransform()));
  }
  if (mat)
  {
    mask.outlineMask.transform(toSkMatrix(*mat));
  }
  return mask;
}

void PathNode::paintEvent(SkCanvas* canvas)
{
  VGG_IMPL(PathNode)
  // if (m_firstChild.empty())
  //   return;
  //
  PaintNode::paintEvent(canvas);
  std::vector<std::pair<SkPath, EBoolOp>> ct;
  for (const auto& c : m_firstChild)
  {
    auto p = static_cast<PaintNode*>(c.get());
    auto outline = p->asOutlineMask(&p->localTransform());
    ct.emplace_back(outline.outlineMask, p->clipOperator());
  }
  if (m_firstChild.empty())
  {
    ct.emplace_back(asOutlineMask(0).outlineMask, EBoolOp::BO_None);
  }

  // draw blur, we assume that there is only one blur style
  bool hasBlur = style.blurs.empty() ? false : style.blurs[0].isEnabled;
  if (hasBlur)
  {
    SkPaint pen;
    pen.setAntiAlias(true);
    const auto blur = style.blurs[0];
    auto sigma = SkBlurMask::ConvertRadiusToSigma(blur.radius);
    if (blur.blurType == BT_Gaussian)
    {
      pen.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));
    }
    else if (blur.blurType == BT_Motion)
    {
      pen.setImageFilter(SkImageFilters::Blur(sigma, 0, nullptr));
    }
    canvas->saveLayer(nullptr, &pen);
  }

  auto mask = makeMaskBy(BO_Intersection);
  if (mask.outlineMask.isEmpty())
  {
    _->drawContour(canvas, m_contextSetting, style, _->windingRule, ct, getBound(), hasFill());
  }
  else
  {
    canvas->save();
    canvas->clipPath(mask.outlineMask);
    _->drawContour(canvas, m_contextSetting, style, _->windingRule, ct, getBound(), hasFill());
    canvas->restore();
  }

  // restore blur
  if (hasBlur)
  {
    canvas->restore();
  }
}

void PathNode::addSubShape(std::shared_ptr<PaintNode> node, EBoolOp op)
{
  node->setClipOperator(op);
  addChild(node);
}

bool PathNode::hasFill() const
{
  for (const auto& f : style.fills)
  {
    if (f.isEnabled)
      return true;
  }
  return false;
}

void PathNode::paintFill(SkCanvas* canvas, float globalAlpha, const SkPath& skPath)
{
  VGG_IMPL(PathNode)
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
      auto gradientShader = _->getGradientShader(f.gradient.value(), getBound());
      fillPen.setShader(gradientShader);
      fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha);
    }
    else if (f.fillType == FT_Pattern)
    {
      assert(f.pattern.has_value());
      auto img = loadImage(f.pattern->imageGUID, Scene::getResRepo());
      if (!img)
        continue;
      auto bs = getBound().size();
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

void PathNode::setWindingRule(EWindingType type)
{
  VGG_IMPL(PathNode)
  _->windingRule = type;
}

PathNode::~PathNode()
{
}
} // namespace VGG
