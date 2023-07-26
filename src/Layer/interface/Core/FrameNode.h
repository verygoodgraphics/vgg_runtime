#pragma once

#include "Common/Config.h"
#include "Core/GroupNode.h"

class SkCanvas;
namespace VGG
{
class VGG_EXPORTS FrameNode final : public PaintNode
{
public:
  FrameNode(const std::string& name);
  Mask asOutlineMask(const glm::mat3* mat) override;
};
} // namespace VGG
