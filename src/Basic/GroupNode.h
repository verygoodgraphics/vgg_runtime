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
        if (m_contextSetting.TransparencyKnockoutGroup)
        {
          SkPaint paint;
          paint.setBlendMode(SkBlendMode::kSrcOver);
          paint.setAlphaf(1.0);
          canvas->save();
          canvas->scale(1, -1);
          canvas->saveLayer(toSkRect(getBound()), &paint);
          p->invokeRenderPass(canvas);
          canvas->restore();
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

  void preRenderPass(SkCanvas* canvas) override
  {
    if (m_contextSetting.Opacity < 1.0)
    {
      // TODO:: more accurate bound is needed
      canvas->saveLayerAlpha(0, m_contextSetting.Opacity * 255);
    }

    if (m_contextSetting.IsolateBlending)
    {
      SkPaint paint;
      paint.setBlendMode(SkBlendMode::kSrc);
      paint.setAlphaf(1.0);
      canvas->save();
      canvas->scale(1, -1);
      canvas->saveLayer(toSkRect(getBound()), &paint);
    }
    PaintNode::preRenderPass(canvas);
  }

  void postRenderPass(SkCanvas* canvas) override
  {
    PaintNode::postRenderPass(canvas);

    if (m_contextSetting.IsolateBlending)
    {
      canvas->restore();
      canvas->restore();
    }

    if (m_contextSetting.Opacity < 1.0)
    {
      canvas->restore();
    }
  }
};
} // namespace VGG
