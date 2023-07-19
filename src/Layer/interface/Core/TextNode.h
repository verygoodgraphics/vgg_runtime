#pragma once
#include "Common/Config.h"
#include "Core/Attrs.h"
#include "Core/FontManager.h"
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
  void setParagraph(std::string utf8,
                    const std::vector<TextAttr>& attrs,
                    const std::vector<TextLineAttr>& lineAttr);
  void setFrameMode(ETextLayoutMode mode);
  void setVerticalAlignment(ETextVerticalAlignment vertAlign);
  ~TextNode();

protected:
  void paintEvent(SkCanvas* canvas) override;
};

} // namespace VGG
