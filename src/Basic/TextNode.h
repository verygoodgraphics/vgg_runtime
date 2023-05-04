#pragma once
#include "Basic/Attrs.h"
#include "PaintNode.h"
#include "VGGType.h"

namespace VGG
{

class TextNode final : public PaintNode
{
  std::string text;
  ETextLayoutMode mode;
  std::vector<TextStyleStub> styles;
  friend class NlohmannBuilder;
public:
  TextNode(const std::string& name, const std::string& text);
  void Paint(SkCanvas* canvas) override;
};

} // namespace VGG
