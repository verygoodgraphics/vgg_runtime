/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "VSkia.hpp"
#include "PaintNodePrivate.hpp"
#include "Layer/Core/ImageNode.hpp"
#include "Layer/Core/Node.hpp"
#include "Layer/Core/PaintNode.hpp"

#include <glm/matrix.hpp>
#include <include/core/SkBlendMode.h>
#include <include/core/SkRect.h>
#include <include/core/SkColor.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathTypes.h>
#include <include/core/SkPoint.h>
#include <include/core/SkTileMode.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkImage.h>

namespace VGG
{

class ImageNode__pImpl
{
  VGG_DECL_API(ImageNode);

public:
  std::string imageGuid;
  bool fillReplacesImage = false;
  sk_sp<SkImage> image;
  sk_sp<SkShader> shader;
  ImageNode__pImpl(ImageNode* api)
    : q_ptr(api)
  {
  }
  ImageNode__pImpl(const ImageNode__pImpl& other)
  {
    imageGuid = other.imageGuid;
    fillReplacesImage = other.fillReplacesImage;
    image = other.image;
    shader = other.shader;
  }
  ImageNode__pImpl& operator=(const ImageNode__pImpl& other) = delete;
  ImageNode__pImpl(ImageNode__pImpl&& other) noexcept = default;
  ImageNode__pImpl& operator=(ImageNode__pImpl&& other) noexcept
  {
    image = std::move(other.image);
    shader = std::move(other.shader);
    imageGuid = std::move(other.imageGuid);
    fillReplacesImage = std::move(other.fillReplacesImage);
    return *this;
  }
};

ImageNode::ImageNode(const std::string& name, std::string guid)
  : PaintNode(name, VGG_IMAGE, std::move(guid))
  , d_ptr(new ImageNode__pImpl(this))
{
}

ImageNode::ImageNode(const ImageNode& other)
  : PaintNode(other)
  , d_ptr(new ImageNode__pImpl(*other.d_ptr))
{
}

Mask ImageNode::asOutlineMask(const glm::mat3* mat)
{
  Mask mask;
  auto rect = toSkRect(getBound());
  mask.outlineMask.addRect(rect);
  if (mat)
  {
    mask.outlineMask.transform(toSkMatrix(*mat));
  }
  return mask;
}

void ImageNode::setImage(const std::string& guid)
{
  VGG_IMPL(ImageNode)
  _->imageGuid = guid;
}

const std::string& ImageNode::getImageGUID() const
{
  return d_ptr->imageGuid;
}

void ImageNode::setReplacesImage(bool fill)
{
  VGG_IMPL(ImageNode)
  _->fillReplacesImage = fill;
}

bool ImageNode::fill() const
{
  return d_ptr->fillReplacesImage;
}

void ImageNode::paintFill(SkCanvas* canvas, SkBlendMode mode, const SkPath& path)
{
  (void)path;
  VGG_IMPL(ImageNode)
  if (!_->image)
  {
    _->image = loadImage(_->imageGuid, Scene::getResRepo());
  }
  if (_->image)
  {
    if (!_->shader)
    {
      const auto& b = getBound();
      _->shader =
        getImageShader(_->image, b.width(), b.height(), EImageFillType::IFT_Stretch, 1.0, false);
    }

    bool hasMask = false;
    if (PaintNode::d_ptr.get()->mask)
    {
      auto mask = PaintNode::d_ptr.get()->mask.value();
      if (mask.isEmpty() == false)
      {
        canvas->save();
        canvas->clipPath(mask);
        hasMask = true;
      }
    }

    SkPaint p;
    p.setShader(_->shader);
    p.setBlendMode(mode);
    canvas->drawPaint(p);
    // Another weird drawing method
    // SkSamplingOptions opt;
    // const auto& b = getBound();
    // SkRect imageRect = SkRect::MakeXYWH(b.topLeft.x, -b.topLeft.y, b.width(), b.height());
    // canvas->save();
    // canvas->scale(1, -1);
    // canvas->drawImageRect(image, imageRect, opt);
    // canvas->restore();

    if (hasMask)
    {
      canvas->restore();
    }
  }
}

ImageNode::~ImageNode() = default;

} // namespace VGG
