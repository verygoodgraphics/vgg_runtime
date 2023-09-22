#pragma once

#include "VGG/Layer/Config.hpp"

#include "VGG/Layer/FontManager.hpp"
#include "VGG/Layer/Core/Node.hpp"
#include "VGG/Layer/Core/PaintNode.hpp"
#include "VGG/Layer/Core/VType.hpp"

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
  void paintEvent(SkiaRenderer* renderer) override;
};

} // namespace VGG
