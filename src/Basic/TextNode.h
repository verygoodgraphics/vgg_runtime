#pragma once
#include "PaintNode.h"
#include "VGGType.h"

namespace VGG
{

class TextNode final : public PaintNode
{
public:
  TextNode(const std::string& name)
    : PaintNode(name, VGG_TEXT)
  {
  }

  void Paint(SkCanvas* canvas) override
  {
    // TODO:: draw text
  }
};

} // namespace VGG
