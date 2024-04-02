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
#include "Layer/AttributeAccessor.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/ImageRenderObjectAttribute.hpp"
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

constexpr bool IMAGE_LEGACY_CODE = false;

class ImageNode__pImpl
{
  VGG_DECL_API(ImageNode);

public:
  bool            fillReplacesImage = false;
  sk_sp<SkShader> shader;
  PatternStretch  pattern;

  ImageAttribtueAccessor* accessor;
  ImageNode::EventHandler imageNodeEventHandler;
  Bounds                  bound;

  ImageNode__pImpl(ImageNode* api)
    : q_ptr(api)
  {
  }
};

ImageNode::ImageNode(VRefCnt* cnt, const std::string& name, std::string guid)
  : PaintNode(cnt, name, VGG_IMAGE, std::move(guid), IMAGE_LEGACY_CODE, false)
  , d_ptr(new ImageNode__pImpl(this))
{

  if (!IMAGE_LEGACY_CODE)
  {
    auto                            t = incRef(transformAttribute());
    Ref<ImageRenderObjectAttribute> ioa;
    auto [c, d] = StyleNode::MakeRenderNode(
      nullptr,
      this,
      t,
      [&](VAllocator* alloc, ObjectAttribute* object) -> Ref<InnerObjectAttribute>
      {
        ioa = ImageRenderObjectAttribute::Make(alloc, object);
        return ioa;
      });

    auto acc = std::make_unique<ImageAttribtueAccessor>(*d, ioa);
    d_ptr->accessor = acc.get();
    PaintNode::d_ptr->accessor = std::move(acc);
    PaintNode::d_ptr->renderNode = std::move(c);
    observe(PaintNode::d_ptr->renderNode);
  }
  // d_ptr->imageNodeEventHandler = [this](ImageAttribtueAccessor* a, void* b)
  // { DEBUG("image node %s", this->name().c_str()); };
}

Bounds ImageNode::getImageBound() const
{
  if (!IMAGE_LEGACY_CODE)
  {
    return d_ptr->accessor->image()->getImageBound();
  }
  else
  {
    return d_ptr->bound;
  }
}

VShape ImageNode::asVisualShape(const Transform* mat)
{
  if (IMAGE_LEGACY_CODE)
  {
    // Mask mask;
    auto rect = toSkRect(getImageBound());
    // mask.outlineMask.addRect(rect);
    if (mat)
    {
      rect = toSkMatrix(mat->matrix()).mapRect(rect);
      // mask.outlineMask.transform(toSkMatrix(mat->matrix()));
    }
    return VShape(rect);
  }
  else
  {
    auto b = toSkRect(d_ptr->accessor->image()->getImageBound());
    return VShape(b);
  }
}

void ImageNode::setImage(const std::string& guid)
{
  VGG_IMPL(ImageNode)
  if (IMAGE_LEGACY_CODE)
  {
    _->pattern.guid = guid;
  }
  else
  {
    d_ptr->accessor->image()->setImageGUID(guid);
  }
}

const std::string& ImageNode::getImageGUID() const
{
  if (IMAGE_LEGACY_CODE)
  {
    return d_ptr->pattern.guid;
  }
  else
  {
    return d_ptr->accessor->image()->getImageGUID();
  }
}

void ImageNode::setImageFilter(const ImageFilter& filter)
{
  if (IMAGE_LEGACY_CODE)
  {
    d_ptr->pattern.imageFilter = filter;
  }
  else
  {
    d_ptr->accessor->image()->setImageFilter(filter);
  }
}

void ImageNode::setReplacesImage(bool fill)
{
  VGG_IMPL(ImageNode)

  if (IMAGE_LEGACY_CODE)
  {
    _->fillReplacesImage = fill;
  }
  else
  {
    DEBUG("deprecated");
  }
}

bool ImageNode::fill() const
{
  if (IMAGE_LEGACY_CODE)
  {
    return d_ptr->fillReplacesImage;
  }
  else
  {
    DEBUG("deprecated");
    return false;
  }
}

void ImageNode::setImageBound(const Bounds& bound)
{
  if (!IMAGE_LEGACY_CODE)
  {
    d_ptr->accessor->image()->setImageBound(bound);
  }
  else
  {
    d_ptr->bound = bound;
  }
}

Bounds ImageNode::onDrawFill(
  Renderer*            renderer,
  sk_sp<SkBlender>     blender,
  sk_sp<SkImageFilter> imageFilter,
  const VShape&        path,
  const VShape&        mask)
{
  if (IMAGE_LEGACY_CODE)
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
      if (!mask.isEmpty())
      {
        canvas->save();
        // canvas->clipPath(mask);
        mask.clip(canvas, SkClipOp::kIntersect);
        hasMask = true;
      }
      SkPaint p;
      p.setShader(_->shader);
      canvas->drawPaint(p);
      if (hasMask)
      {
        canvas->restore();
      }
    }
    return bound();
  }
  else
  {
    ASSERT(false && "unreachable");
    return bound();
  }
}

void ImageNode::installImageNodeEventHandler(EventHandler handler)
{
  d_ptr->imageNodeEventHandler = std::move(handler);
}

void ImageNode::dispatchEvent(void* event)
{
  if (d_ptr->imageNodeEventHandler)
  {
    d_ptr->imageNodeEventHandler(static_cast<ImageAttribtueAccessor*>(attributeAccessor()), event);
  }
}

ImageNode::~ImageNode() = default;

} // namespace VGG::layer
