/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#pragma once
#include "AttributeNode.hpp"
#include "ShapeAttribute.hpp"
#include "GraphicItem.hpp"
#include "ObjectAttribute.hpp"

#include "Layer/Core/Attrs.hpp"
#include <any>

namespace VGG::layer
{
class ImageItem final : public GraphicItem
{
public:
  ImageItem(VRefCnt* cnt, ObjectAttribute* objectAttribute);
  void render(Renderer* renderer) override;

  sk_sp<SkImageFilter> getMaskFilter() const override
  {
    return 0;
  }

  ShapeAttribute* shape() const override
  {
    return m_imageShape.get();
  }

  void setImageGUID(const std::string& guid)
  {
    if (guid == m_imagePattern.guid)
      return;
    m_imagePattern.guid = guid;
    this->invalidate();
  }

  const std::string& getImageGUID() const
  {
    return m_imagePattern.guid;
  }

  void setImageFilter(const ImageFilter& filter)
  {
    if (filter == m_imagePattern.imageFilter)
      return;
    m_imagePattern.imageFilter = filter;
    this->invalidate();
  }

  const ImageFilter& getImageFilter() const
  {
    return m_imagePattern.imageFilter;
  }

  void setImageBounds(const Bounds& bounds)
  {
    if (m_imageBounds == bounds)
      return;
    ASSERT(m_imageShape);
    m_imageBounds = bounds;
    m_imageShape->setShape(VShape(toSkRect(bounds)));
  }

  const Bounds& getImageBounds() const
  {
    return m_imageBounds;
  }

  Bounds effectBounds() const override
  {
    // FIXME: border should be considered
    return m_imageBounds;
  }

  VGG_CLASS_MAKE(ImageItem);
  Bounds onRevalidate() override;

private:
  WeakRef<ObjectAttribute> m_objectAttribute;
  Ref<ShapeAttributeImpl>  m_imageShape;
  sk_sp<SkShader>          m_imageShader;
  PatternStretch           m_imagePattern;
  Bounds                   m_imageBounds;
};
} // namespace VGG::layer
