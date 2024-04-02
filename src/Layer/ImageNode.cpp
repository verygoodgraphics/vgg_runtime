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
#include "ImageGraphicItem.hpp"
#include "Renderer.hpp"
#include "StyleItem.hpp"
#include "VSkia.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/ImageNode.hpp"
#include "Layer/Core/PaintNode.hpp"

namespace VGG::layer
{

class ImageNode__pImpl
{
  VGG_DECL_API(ImageNode);

public:
  bool            fillReplacesImage = false;
  sk_sp<SkShader> shader;
  PatternStretch  pattern;

  ImageItemAttribtueAccessor* accessor;
  ImageNode::EventHandler     imageNodeEventHandler;
  Bounds                      bound;

  ImageNode__pImpl(ImageNode* api)
    : q_ptr(api)
  {
  }
};

ImageNode::ImageNode(VRefCnt* cnt, const std::string& name, std::string guid)
  : PaintNode(cnt, name, VGG_IMAGE, std::move(guid), false)
  , d_ptr(new ImageNode__pImpl(this))
{

  auto           t = incRef(transformAttribute());
  Ref<ImageItem> ioa;
  auto [c, d] = StyleItem::MakeRenderNode(
    nullptr,
    this,
    t,
    [&](VAllocator* alloc, ObjectAttribute* object) -> Ref<GraphicItem>
    {
      ioa = ImageItem::Make(alloc, object);
      return ioa;
    });

  auto acc = std::make_unique<ImageItemAttribtueAccessor>(*d, ioa);
  d_ptr->accessor = acc.get();
  onSetAccessor(std::move(acc));
  onSetStyleItem(c);
  observe(std::move(c));
}

Bounds ImageNode::getImageBound() const
{
  return d_ptr->accessor->image()->getImageBounds();
}

VShape ImageNode::asVisualShape(const Transform* mat)
{
  auto b = toSkRect(d_ptr->accessor->image()->getImageBounds());
  return VShape(b);
}

void ImageNode::setImage(const std::string& guid)
{
  d_ptr->accessor->image()->setImageGUID(guid);
}

const std::string& ImageNode::getImageGUID() const
{
  return d_ptr->accessor->image()->getImageGUID();
}

void ImageNode::setImageFilter(const ImageFilter& filter)
{
  d_ptr->accessor->image()->setImageFilter(filter);
}

void ImageNode::setImageBound(const Bounds& bound)
{
  d_ptr->accessor->image()->setImageBounds(bound);
}

void ImageNode::installImageNodeEventHandler(EventHandler handler)
{
  d_ptr->imageNodeEventHandler = std::move(handler);
}

void ImageNode::dispatchEvent(void* event)
{
  if (d_ptr->imageNodeEventHandler)
  {
    d_ptr->imageNodeEventHandler(
      static_cast<ImageItemAttribtueAccessor*>(attributeAccessor()),
      event);
  }
}

ImageNode::~ImageNode() = default;

} // namespace VGG::layer
