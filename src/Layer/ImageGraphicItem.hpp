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
#include "PenNode.hpp"

#include "Layer/Core/Attrs.hpp"
#include <any>
#include <variant>

namespace VGG::layer
{
class ImageItem final : public GraphicItem
{
public:
  ImageItem(VRefCnt* cnt, StyleItem* objectAttribute);
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
    Pattern        patt;
    PatternStretch ps;
    ps.guid = guid;
    ps.imageFilter = getImageFilter();
    patt.instance = ps;
    m_brush->setBrush(std::move(patt));
  }

  const std::string& getImageGUID() const
  {
    return pattern().guid;
  }

  void setImageFilter(const ImageFilter& filter)
  {
    Pattern        patt;
    PatternStretch ps;
    ps.guid = getImageGUID();
    ps.imageFilter = filter;
    patt.instance = ps;
    m_brush->setBrush(std::move(patt));
  }

  const ImageFilter& getImageFilter() const
  {
    return pattern().imageFilter;
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
  Bounds onRevalidate(Revalidation* inv, const glm::mat3& mat) override;

private:
  const PatternStretch& pattern() const
  {
    return std::get<PatternStretch>(std::get<Pattern>(m_brush->getBrush()).instance);
  }

  Ref<ShapeAttributeImpl> m_imageShape;
  Bounds                  m_imageBounds;
  Ref<Brush>              m_brush;
};
} // namespace VGG::layer
