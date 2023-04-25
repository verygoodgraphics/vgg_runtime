#pragma once
#include "nlohmann/json.hpp"
#include "Scene.hpp"
#include "PaintNode.h"
#include "VGGUtils.h"
#include "VGGType.h"
#include <tuple>
#include <unordered_map>

namespace VGG
{

// Node that has transform and bound should be defined as tree node
//
struct TraversalData
{
  int depth = 0;
  glm::mat3 transform;
};

inline SkCanvas* PaintNode::s_defaultCanvas = nullptr;

class ArtboardNode final : public PaintNode
{
public:
  ArtboardNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_ARTBOARD)
  {
  }
};

} // namespace VGG
