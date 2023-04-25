#pragma once
#include "PaintNode.h"
#include "VGGType.h"

namespace VGG
{
class ImageNode final : public PaintNode
{
  std::string guid;
  bool fillReplacesImage = false;

public:
  ImageNode(const std::string& name)
    : PaintNode(name, VGG_IMAGE)
  {
  }

  void Paint(SkCanvas* canvas) override
  {
    // TODO:: draw image
  }

  void setImage(const std::string& guid)
  {
    this->guid = guid;
  }

  const std::string& getImageGUID() const
  {
    return this->guid;
  }

  void setReplacesImage(bool fill)
  {
    this->fillReplacesImage = fill;
  }

  bool fill() const
  {
    return this->fillReplacesImage;
  }
};
} // namespace VGG
