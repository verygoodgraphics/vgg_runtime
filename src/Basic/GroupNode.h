#pragma once

#include "PaintNode.h"

namespace VGG
{
class GroupNode final : public PaintNode
{
public:
  GroupNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_GROUP)
  {
  }
};
} // namespace VGG
