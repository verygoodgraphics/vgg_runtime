#pragma once
#include "VGG/Layer/Core/Node.hpp"
#include "VGG/Layer/Core/PaintNode.hpp"
#include "VGG/Layer/Config.hpp"

class SkImage;
class SkCanvas;

namespace VGG
{
class ImageNode__pImpl;
class VGG_EXPORTS ImageNode final : public PaintNode
{
  VGG_DECL_IMPL(ImageNode)
public:
  ImageNode(const std::string& name, std::string guid);
  ImageNode(const ImageNode&);
  ImageNode& operator=(const ImageNode&) = delete;

  ImageNode(ImageNode&&) noexcept = default;
  ImageNode& operator=(ImageNode&&) noexcept = default;
  // void paintEvent(SkCanvas* canvas) override;
  void setImage(const std::string& guid);
  const std::string& getImageGUID() const;
  void setReplacesImage(bool fill);
  bool fill() const;
  Mask asOutlineMask(const glm::mat3* mat) override;

  virtual ~ImageNode() override;

protected:
  void paintFill(SkCanvas* canvas, const SkPath& path) override;
};
} // namespace VGG
