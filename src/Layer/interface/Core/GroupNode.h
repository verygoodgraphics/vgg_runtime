#pragma once

#include "Common/Config.h"
#include "Core/PaintNode.h"

class SkCanvas;
namespace VGG
{
class VGG_EXPORTS GroupNode : public PaintNode
{
public:
  GroupNode(const std::string& name);
  Mask asOutlineMask(const glm::mat3* mat) override;
  void renderOrderPass(SkCanvas* canvas) override;
  void preRenderPass(SkCanvas* canvas) override;
  void postRenderPass(SkCanvas* canvas) override;
};
} // namespace VGG
