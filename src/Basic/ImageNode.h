#pragma once
#include "Basic/PathNode.h"
#include "PaintNode.h"

class SkImage;

namespace VGG
{
class ImageNode final : public PaintNode
{
  std::string guid;
  bool fillReplacesImage = false;
  sk_sp<SkImage> image;

public:
  ImageNode(const std::string& name);
  void paintEvent(SkCanvas* canvas) override;
  void setImage(const std::string& guid);
  const std::string& getImageGUID() const;
  void setReplacesImage(bool fill);
  bool fill() const;
};
} // namespace VGG
