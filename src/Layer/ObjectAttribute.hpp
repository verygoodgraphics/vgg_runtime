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
#include "GraphicItem.hpp"
#include "ObjectShader.hpp"
#include "ShadowAttribute.hpp"

#include "Layer/Core/Attrs.hpp"

namespace VGG::layer
{

class BackdropFXAttribute;

class ObjectAttribute : public Attribute
{
public:
  ObjectAttribute(VRefCnt* cnt, Ref<GraphicItem> renderObject)
    : Attribute(cnt)
    , m_graphicItem(renderObject)
  {
    observe(m_graphicItem);
  }

  bool hasFill() const
  {
    return m_hasFill;
  }

  sk_sp<SkImageFilter> imageFilter() const
  {
    return 0;
  }

  sk_sp<SkBlender> blender() const
  {
    return 0;
  }

  Bounds effectBounds() const
  {
    ASSERT(m_graphicItem);
    // TODO:: bounds with border should evaluated here rather than in concrete graphics item
    return m_graphicItem->effectBounds();
  }

  void render(Renderer* renderer);

  Bounds onRevalidate(Revalidation* inv, const glm::mat3 & mat) override;

  VGG_ATTRIBUTE(FillStyle, const std::vector<Fill>&, m_fills);
  VGG_ATTRIBUTE(BorderStyle, const std::vector<Border>&, m_borders);
  VGG_ATTRIBUTE(GraphicItem, Ref<GraphicItem>, m_graphicItem);

  VGG_CLASS_MAKE(ObjectAttribute);

private:
  friend class RenderNode;
  std::pair<SkRect, std::optional<SkPaint>> revalidateObjectBounds(
    const std::vector<Border>& borders,
    const SkRect&              bounds);
  void revalidateMaskFilter(const SkPaint& paint, const SkRect& bounds);

  Ref<GraphicItem>    m_graphicItem;
  std::vector<Fill>   m_fills;
  std::vector<Border> m_borders;
  bool                m_hasFill{ false };
};

} // namespace VGG::layer
