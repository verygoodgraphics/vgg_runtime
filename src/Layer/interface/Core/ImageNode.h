#pragma once
#include "Common/Config.h"
#include "Core/PathNode.h"
#include "Core/PaintNode.h"

class SkImage;
class SkCanvas;

namespace VGG
{
class VGG_EXPORTS ImageNode final : public PathNode
{
  std::string guid;
  bool fillReplacesImage = false;
  sk_sp<SkImage> image;
  sk_sp<SkShader> shader;

public:
  ImageNode(const std::string& name, std::string guid);
  // void paintEvent(SkCanvas* canvas) override;
  void setImage(const std::string& guid);
  const std::string& getImageGUID() const;
  void setReplacesImage(bool fill);
  bool fill() const;
  Mask asOutlineMask(const glm::mat3* mat) override;

protected:
  void paintFill(SkCanvas* canvas,
                 float globalAlpha,
                 const Style& style,
                 const SkPath& skPath,
                 const Bound2& bound) override;
};
} // namespace VGG
