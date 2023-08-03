#pragma once
#include "Common/Config.h"
#include "Core/Attrs.h"
#include "Core/FontManager.h"
#include "Core/Node.h"
#include "Core/PaintNode.h"
#include "Core/VType.h"

namespace VGG
{

class TextNode__pImpl;
class VGG_EXPORTS TextNode final : public PaintNode
{
  VGG_DECL_IMPL(TextNode);
  friend class NlohmannBuilder;

public:
  TextNode(const std::string& name, std::string guid);
  TextNode(const TextNode&);
  TextNode& operator=(const TextNode&) = delete;
  TextNode(TextNode&&) noexcept = default;
  TextNode& operator=(TextNode&&) noexcept = default;
  void setParagraph(std::string utf8,
                    const std::vector<TextAttr>& attrs,
                    const std::vector<TextLineAttr>& lineAttr);
  void setFrameMode(ETextLayoutMode mode);
  void setVerticalAlignment(ETextVerticalAlignment vertAlign);
  NodePtr clone() const override;
  ~TextNode();

protected:
  void paintEvent(SkCanvas* canvas) override;
};

} // namespace VGG
