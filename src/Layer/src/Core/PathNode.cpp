#include "Core/Attrs.h"
#include "Core/ContourNode.h"
#include "Core/PaintNode.h"
#include "Core/PathNode.h"
#include "Core/VType.h"
#include "Core/PathNodePrivate.h"
#include "SkiaImpl/VSkia.h"

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

PathNode::PathNode(const std::string& name, std::string guid)
  : PaintNode(name, ObjectType::VGG_PATH, std::move(guid))
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

SkPath PathNode::getContour()
{
  VGG_IMPL(PathNode)
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
  SkPath skPath = _->makePath(ct);
  return skPath;
}

void PathNode::paintEvent(SkCanvas* canvas)
{
  VGG_IMPL(PathNode)
  // if (m_firstChild.empty())
  //   return;
  //
  // paintBackgroundColor(canvas); // Is it necessary?

  SkPath skPath = getContour();
  skPath.setFillType(_->windingRule == EWindingType::WR_EvenOdd ? SkPathFillType::kEvenOdd
                                                                : SkPathFillType::kWinding);
  // draw blur, we assume that there is only one blur style
  bool hasBlur = style().blurs.empty() ? false : style().blurs[0].isEnabled;
  if (hasBlur)
  {
    auto pen = _->makeBlurPen(style().blurs[0]);
    canvas->saveLayer(nullptr, &pen);
  }

  auto mask = makeMaskBy(BO_Intersection);
  if (mask.outlineMask.isEmpty())
  {
    _->drawContour(canvas, contextSetting(), style(), skPath, getBound());
  }
  else
  {
    canvas->save();
    canvas->clipPath(mask.outlineMask);
    _->drawContour(canvas, contextSetting(), style(), skPath, getBound());
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

void PathNode::addSubShape(ContourPtr contour, EBoolOp op)
{
}

void PathNode::paintFill(SkCanvas* canvas,
                         float globalAlpha,
                         const Style& style,
                         const SkPath& skPath,
                         const Bound2& bound)
{
  d_ptr->drawFill(canvas, globalAlpha, style, skPath, bound);
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
