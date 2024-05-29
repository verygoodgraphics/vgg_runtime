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
#include "ShapeAttribute.hpp"
#include "AttributeNode.hpp"
#include "ShadowEffects.hpp"

namespace VGG::layer
{

class ShapeAttribute;

class DropShadowAttribute : public Attribute
{
public:
  DropShadowAttribute(VRefCnt* cnt, Ref<ShapeAttribute> shapeAttr)
    : Attribute(cnt)
    , m_shapeAttr(shapeAttr)
  {
    observe(m_shapeAttr);
  }

  void   render(Renderer* renderer);
  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override;

  VGG_ATTRIBUTE(DropShadowStyle, const std::vector<DropShadow>&, m_shadow);
  VGG_CLASS_MAKE(DropShadowAttribute);

private:
  friend class RenderNode;
  Ref<ShapeAttribute>             m_shapeAttr;
  std::vector<DropShadow>         m_shadow;
  std::optional<DropShadowEffect> m_dropShadowEffects;
};

class InnerShadowAttribute : public Attribute
{
public:
  InnerShadowAttribute(VRefCnt* cnt, Ref<ShapeAttribute> shapeAttr)
    : Attribute(cnt)
    , m_shapeAttr(shapeAttr)
  {
    observe(m_shapeAttr);
  }
  void   render(Renderer* renderer);
  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override;
  VGG_ATTRIBUTE(InnerShadowStyle, const std::vector<InnerShadow>&, m_shadow);
  VGG_CLASS_MAKE(InnerShadowAttribute);

private:
  friend class RenderNode;
  Ref<ShapeAttribute>              m_shapeAttr;
  std::vector<InnerShadow>         m_shadow;
  std::optional<InnerShadowEffect> m_innerShadowEffects;
};
} // namespace VGG::layer
