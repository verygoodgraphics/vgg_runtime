#pragma once
#include "Basic/RenderState.h"
#include "Basic/PaintNode.h"
#include "nlohmann/json.hpp"
#include "Scene.hpp"
#include "PaintNode.h"
#include "VGGUtils.h"
#include "VGGType.h"
#include <tuple>
#include <unordered_map>

namespace VGG
{

class ArtboardNode final : public PaintNode
{
public:
  ArtboardNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_ARTBOARD)
  {
  }
};

} // namespace VGG
