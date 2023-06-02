#include "Basic/Attrs.h"
#include "Basic/ContourNode.h"
#include "Basic/PaintNode.h"
#include "Basic/PathNode.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "Basic/SkiaBackend/SkiaImpl.h"
#include "Basic/PathNodePrivate.h"

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

  // draw blur
  bool hasBlur = style.blurs.empty() ? false : style.blurs[0].isEnabled;
  if (hasBlur)
  {
    SkPaint pen;
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
    if (f.fillType == FT_Color)
    {
      fillPen.setColor(f.color);
      const auto currentAlpha = fillPen.getAlphaf();
      fillPen.setAlphaf(f.contextSettings.Opacity * globalAlpha * currentAlpha);
    }
    else if (f.fillType == FT_Gradient)
    {
      assert(f.gradient.has_value());
      auto gradientShader = _->getGradientShader(f.gradient.value(), getBound().size());
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
