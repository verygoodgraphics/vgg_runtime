#pragma once

#include "Basic/VGGType.h"
#include "Basic/VGGUtils.h"
#include "PaintNode.h"
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

  Mask asOutlineMask(const SkMatrix* mat) override
  {
    Mask mask;
    if (!hasChild())
    {
      mask.outlineMask.addRect(toSkRect(bound));
    }
    else
    {
      for (const auto& c : m_firstChild)
      {
        auto paintNode = static_cast<PaintNode*>(c.get());
        const auto m = toSkMatrix(paintNode->localTransform());
        auto childMask = paintNode->asOutlineMask(&m);
        Op(mask.outlineMask, childMask.outlineMask, SkPathOp::kUnion_SkPathOp, &mask.outlineMask);
      }
    }
    if (mat)
    {
      mask.outlineMask.transform(*mat);
    }
    return mask;
  }
  void traverse() override
  {
    preVisit();

    // deal with mask rendering order
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
    for (const auto& p : masked)
      p->traverse();
    for (const auto& p : noneMasked)
      p->traverse();
    postVisit();
  }
};
} // namespace VGG
