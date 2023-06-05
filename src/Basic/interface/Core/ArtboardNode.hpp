#pragma once
#include "Common/Config.h"
#include "Core/RenderState.h"
#include "Core/PaintNode.h"
#include "nlohmann/json.hpp"
#include "Core/PaintNode.h"
#include "Core/VGGUtils.h"
#include "Core/VGGType.h"
#include "Scene/Scene.h"
#include <tuple>
#include <unordered_map>

namespace VGG
{

class VGG_EXPORTS ArtboardNode final : public PaintNode
{
public:
  ArtboardNode(const std::string& name)
    : PaintNode(name, ObjectType::VGG_ARTBOARD)
  {
  }
};

} // namespace VGG
