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
#pragma once
#include "AttributeNode.hpp"
#include "RenderObjectAttribute.hpp"
#include "ObjectShader.hpp"
#include "ShadowAttribute.hpp"

#include "Layer/Core/Attrs.hpp"

namespace VGG::layer
{

class BackdropFXAttribute;

class ObjectAttribute : public Attribute
{
public:
  ObjectAttribute(VRefCnt* cnt, Ref<InnerObjectAttribute> renderObject)
    : Attribute(cnt)
    , m_renderObjectAttr(renderObject)
  {
    observe(m_renderObjectAttr);
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
    ASSERT(m_renderObjectAttr);
    return m_renderObjectAttr->effectBounds();
  }

  void   render(Renderer* renderer);
  Bounds onRevalidate() override;

  VGG_ATTRIBUTE(FillStyle, std::vector<Fill>, m_fills);
  VGG_ATTRIBUTE(BorderStyle, std::vector<Border>, m_borders);
  VGG_ATTRIBUTE(RenderObject, Ref<InnerObjectAttribute>, m_renderObjectAttr);

  VGG_CLASS_MAKE(ObjectAttribute);

private:
  friend class RenderNode;
  std::pair<SkRect, std::optional<SkPaint>> revalidateObjectBounds(
    const std::vector<Border>& borders,
    const SkRect&              bounds);
  void revalidateMaskFilter(const SkPaint& paint, const SkRect& bounds);

  Ref<InnerObjectAttribute> m_renderObjectAttr;
  sk_sp<SkImageFilter>       m_maskFilter;
  std::vector<Fill>          m_fills;
  std::vector<Border>        m_borders;
  bool                       m_hasFill{ false };
};

class StyleAttribute : public Attribute
{
public:
  StyleAttribute(
    VRefCnt*                  cnt,
    Ref<InnerShadowAttribute> innerShadow,
    Ref<DropShadowAttribute>  dropShadow,
    Ref<ObjectAttribute>      object,
    Ref<BackdropFXAttribute>  backgroundBlur);

  void render(Renderer* renderer);

  sk_sp<SkImageFilter> getBackdropImageFilter() const
  {
    return m_dropbackImageFilter;
  }

  Bounds effectBounds() const
  {
    return m_effectBounds;
  }

  Bounds onRevalidate() override;

  VGG_CLASS_MAKE(StyleAttribute);

private:
  friend class RenderNode;

  void revalidateDropbackFilter(const SkRect& bounds);

  Ref<InnerShadowAttribute> m_innerShadowAttr;
  Ref<DropShadowAttribute>  m_dropShadowAttr;
  Ref<ObjectAttribute>      m_objectAttr;
  Ref<BackdropFXAttribute>  m_backgroundBlurAttr;

  SkRect               m_objectEffectBounds;
  Bounds               m_effectBounds;
  sk_sp<SkImageFilter> m_bgBlurImageFilter;

  sk_sp<SkImageFilter> m_dropbackImageFilter;
  sk_sp<SkImageFilter> m_objectImageFilter;
};
} // namespace VGG::layer
