#include <Core/GroupNode.h>

#include "Core/VGGType.h"
#include "core/SkCanvas.h"

namespace VGG
{

GroupNode::GroupNode(const std::string& name)
  : PaintNode(name, ObjectType::VGG_GROUP)
{
}

Mask GroupNode::asOutlineMask(const glm::mat3* mat)
{
  Mask mask;
  if (!hasChild())
  {
    mask.outlineMask.addRect(toSkRect(getBound()));
  }
  else
  {
    for (const auto& c : m_firstChild)
    {
      auto paintNode = static_cast<PaintNode*>(c.get());
      auto childMask = paintNode->asOutlineMask(&paintNode->localTransform());
      Op(mask.outlineMask, childMask.outlineMask, SkPathOp::kUnion_SkPathOp, &mask.outlineMask);
    }
  }
  if (mat)
  {
    mask.outlineMask.transform(toSkMatrix(*mat));
  }
  return mask;
}

void GroupNode::renderOrderPass(SkCanvas* canvas)
{
  // deal with mask rendering order
  //
  std::vector<PaintNode*> masked;
  std::vector<PaintNode*> noneMasked;
  for (const auto& p : this->m_firstChild)
  {
    auto c = static_cast<PaintNode*>(p.get());
    if (c->getMaskType() == MT_Outline)
      masked.push_back(c);
    else
      noneMasked.push_back(c);
  }

  auto paintCall = [&](std::vector<PaintNode*>& nodes)
  {
    if (m_contextSetting.TransparencyKnockoutGroup)
    {
      for (const auto& p : nodes)
      {
        // TODO:: blend mode r = s!=0?s:d is needed.
        // SkPaint paint;
        // paint.setBlendMode(SkBlendMode::kSrc);
        // canvas->save();
        // canvas->scale(1, -1);
        // canvas->saveLayer(toSkRect(getBound()), &paint);
        p->invokeRenderPass(canvas);
        // canvas->restore();
        // canvas->restore();
      }
    }
    else
    {
      for (const auto& p : nodes)
      {
        p->invokeRenderPass(canvas);
      }
    }
  };

  paintCall(masked);
  paintCall(noneMasked);
}

void GroupNode::preRenderPass(SkCanvas* canvas)
{
  if (m_contextSetting.Opacity < 1.0)
  {
    // TODO:: more accurate bound is needed
    canvas->saveLayerAlpha(0, m_contextSetting.Opacity * 255);
  }

  if (m_contextSetting.IsolateBlending)
  {
    // TODO:: blend mode r = s!=0?s:d is needed.
    // SkPaint paint;
    // paint.setBlendMode(SkBlendMode::kSrc);
    // canvas->save();
    // canvas->scale(1, -1);
    // canvas->saveLayer(toSkRect(getBound()), &paint);
  }
  PaintNode::preRenderPass(canvas);
}

void GroupNode::postRenderPass(SkCanvas* canvas)
{
  PaintNode::postRenderPass(canvas);

  if (m_contextSetting.IsolateBlending)
  {
    // canvas->restore();
    // canvas->restore();
  }

  if (m_contextSetting.Opacity < 1.0)
  {
    canvas->restore();
  }
}
}; // namespace VGG
