#pragma once
#include "Core/Attrs.h"
#include "Core/PaintNode.h"
#include "Core/VGGType.h"

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
  void paintEvent(SkCanvas* canvas) override;
};

} // namespace VGG
