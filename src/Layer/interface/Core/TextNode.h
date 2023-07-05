#pragma once
#include "Common/Config.h"
#include "Core/Attrs.h"
#include "Core/Node.hpp"
#include "Core/PaintNode.h"
#include "Core/VGGType.h"

namespace VGG
{

class TextNode__pImpl;
class VGG_EXPORTS TextNode final : public PaintNode
{
  VGG_DECL_IMPL(TextNode);
  friend class NlohmannBuilder;

public:
  TextNode(const std::string& name);
  void setText(const std::string& utf8, const std::vector<TextStyleStub>& styles);
  void setTextStyle(size_t position, const TextStyleStub& style);
  void setFrameMode(ETextLayoutMode mode);
  ~TextNode();

protected:
  void paintEvent(SkCanvas* canvas) override;
};

} // namespace VGG
