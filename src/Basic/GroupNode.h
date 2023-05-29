#pragma once

#include "Basic/SkiaBackend/SkiaImpl.h"
#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "PaintNode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"

namespace VGG
{
class GroupNode final : public PaintNode
{
public:
  GroupNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_GROUP)
  {
  }

  Mask asOutlineMask(const glm::mat3* mat) override
  {
    Mask mask;
    if (!hasChild())
    {
      mask.outlineMask.addRect(toSkRect(m_bound));
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

  void renderOrderPass(SkCanvas* canvas) override
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
      for (const auto& p : nodes)
      {
        if (contextSetting.TransparencyKnockoutGroup)
        {
          SkPaint paint;
          paint.setBlendMode(SkBlendMode::kSrc);
          paint.setAlphaf(1.0);
          // TODO:: bound
          canvas->saveLayer(0, &paint);
          p->invokeRenderPass(canvas);
          canvas->restore();
        }
        else
        {
          p->invokeRenderPass(canvas);
        }
      }
    };

    paintCall(masked);
    paintCall(noneMasked);
  }

  void paintEvent(SkCanvas* canvas) override
  {
    if (contextSetting.IsolateBlending)
    {
      SkPaint paint;
      paint.setBlendMode(SkBlendMode::kSrc);
      // TODO:: bound
      canvas->saveLayer(toSkRect(getBound()), &paint);
      PaintNode::paintEvent(canvas);
      canvas->restore();
    }
    else
    {
      PaintNode::paintEvent(canvas);
    }
  }
};
} // namespace VGG
