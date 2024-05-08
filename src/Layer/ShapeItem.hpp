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
#include "Layer/AttributeNode.hpp"
#include "Layer/ObjectAttribute.hpp"
#include "Layer/ShapeAttribute.hpp"
#include "GraphicItem.hpp"
#include "ObjectShader.hpp"

namespace VGG::layer
{

class ObjectAttribute;
class ShapeItem : public GraphicItem
{
public:
  ShapeItem(VRefCnt* cnt, Ref<ShapeAttribute> shapeAttr)
    : GraphicItem(cnt)
    , m_shapeAttr(std::move(shapeAttr))
  {
    observe(m_shapeAttr);
  }
  ShapeItem(VRefCnt* cnt, Ref<ShapeAttribute> shapeAttr, ObjectAttribute* objectAttribute)
    : GraphicItem(cnt)
    , m_objectAttribute(objectAttribute)
    , m_shapeAttr(std::move(shapeAttr))
  {
    observe(m_shapeAttr);
  }
  void            render(Renderer* renderer) override;
  ShapeAttribute* shape() const override;

  Bounds effectBounds() const override
  {
    return m_effectBounds;
  }

  sk_sp<SkImageFilter> getMaskFilter() const override
  {
    return m_maskFilter;
  }

  VGG_ATTRIBUTE(ObjectAttribute, WeakRef<ObjectAttribute>, m_objectAttribute);
  Bounds onRevalidate() override;
  VGG_CLASS_MAKE(ShapeItem);

private:
  std::pair<SkRect, std::optional<SkPaint>> revalidateObjectBounds(
    const std::vector<Border>& borders,
    const SkRect&              bounds);
  WeakRef<ObjectAttribute>    m_objectAttribute;
  Ref<ShapeAttribute>         m_shapeAttr;
  sk_sp<SkImageFilter>        m_maskFilter;
  Bounds                      m_effectBounds;
  std::optional<ObjectShader> m_objectShader;
};

} // namespace VGG::layer
