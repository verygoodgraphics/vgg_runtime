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
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Renderer.hpp"
#include "VSkia.hpp"
#include "PaintNodePrivate.hpp"
#include "Layer/Core/ImageNode.hpp"
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

namespace VGG::layer
{

class ImageNode__pImpl
{
  VGG_DECL_API(ImageNode);

public:
  bool            fillReplacesImage = false;
  sk_sp<SkShader> shader;
  PatternStretch  pattern;
  ImageNode__pImpl(ImageNode* api)
    : q_ptr(api)
  {
  }
};

ImageNode::ImageNode(VRefCnt* cnt, const std::string& name, std::string guid)
  : PaintNode(cnt, name, VGG_IMAGE, std::move(guid))
  , d_ptr(new ImageNode__pImpl(this))
{
}

Shape ImageNode::asOutlineMask(const Transform* mat)
{
  // Mask mask;
  auto rect = toSkRect(frameBound());
  // mask.outlineMask.addRect(rect);
  if (mat)
  {
    rect = toSkMatrix(mat->matrix()).mapRect(rect);
    // mask.outlineMask.transform(toSkMatrix(mat->matrix()));
  }
  return Shape(rect);
}

void ImageNode::setImage(const std::string& guid)
{
  VGG_IMPL(ImageNode)
  _->pattern.guid = guid;
}

const std::string& ImageNode::getImageGUID() const
{
  return d_ptr->pattern.guid;
}

void ImageNode::setImageFilter(const ImageFilter& filter)
{
  d_ptr->pattern.imageFilter = filter;
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

void ImageNode::paintFill(
  Renderer*            renderer,
  sk_sp<SkBlender>     blender,
  sk_sp<SkImageFilter> imageFilter,
  const Shape&         path)
{
  (void)path;
  VGG_IMPL(ImageNode)
  auto canvas = renderer->canvas();
  if (!_->shader)
  {
    _->shader = makeStretchPattern(bound(), d_ptr->pattern);
  }
  if (_->shader)
  {
    bool hasMask = false;
    if (PaintNode::d_ptr.get()->mask)
    {
      auto mask = PaintNode::d_ptr.get()->mask.value();
      if (mask.isEmpty() == false)
      {
        canvas->save();
        // canvas->clipPath(mask);
        mask.clip(canvas, SkClipOp::kIntersect);
        hasMask = true;
      }
    }
    SkPaint p;
    p.setShader(_->shader);
    canvas->drawPaint(p);
    if (hasMask)
    {
      canvas->restore();
    }
  }
}

ImageNode::~ImageNode() = default;

} // namespace VGG::layer
