#pragma once

#include "Core/Node.h"
#include "Core/VType.h"
#include "Core/PathNode.h"
#include "core/SkShader.h"

namespace VGG
{
class PathNode__pImpl
{
  VGG_DECL_API(PathNode);

public:
  EWindingType windingRule;
  PathNode__pImpl(PathNode* api)
    : q_ptr(api)
  {
  }

  SkPath makePath(const std::vector<std::pair<SkPath, EBoolOp>>& ct);
};
} // namespace VGG
