#pragma once

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

  SkPath makeOutlineMask(const glm::mat3* mat) override
  {
    SkPath p;
    if (!hasChild())
    {
      p.addRect(toSkRect(bound));
    }
    else
    {
      for (const auto& c : m_firstChild)
      {
        auto paintNode = static_cast<PaintNode*>(c.get());
        auto childPath = paintNode->makeOutlineMask(&this->transform);
        Op(p, childPath, SkPathOp::kUnion_SkPathOp, &p);
      }
    }
    if (mat)
    {
      p.transform(toSkMatrix(*mat));
    }
    return p;
  }
};
} // namespace VGG
