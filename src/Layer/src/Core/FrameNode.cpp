#include "Core/FrameNode.h"
#include "Core/PaintNode.h"
#include "Core/VType.h"

namespace VGG
{
FrameNode::FrameNode(const std::string& name)
  : PaintNode(name, VGG_FRAME)
{
}

Mask FrameNode::asOutlineMask(const glm::mat3* mat)
{
  Mask m;
  return m;
}

} // namespace VGG
