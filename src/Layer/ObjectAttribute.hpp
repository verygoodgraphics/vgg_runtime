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
#include "ImageFilterAttribute.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/RenderObjectAttribute.hpp"
#include "ShapeAttribute.hpp"
#include "ShadowEffects.hpp"
#include "ObjectShader.hpp"

namespace VGG::layer
{

class ObjectAttribute;

class BackgroundBlurAttribute : public ImageFilterAttribute
{
public:
  BackgroundBlurAttribute(VRefCnt* cnt)
    : ImageFilterAttribute(cnt)
  {
  }
  VGG_ATTRIBUTE(BackgroundBlur, std::vector<BackgroundFX>, m_blurs);
  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_imageFilter;
  }
  Bound onRevalidate() override;
  VGG_CLASS_MAKE(BackgroundBlurAttribute);

private:
  friend class RenderNode;
  std::vector<BackgroundFX> m_blurs;
  sk_sp<SkImageFilter>      m_imageFilter;
};

class DropShadowAttribute : public Attribute
{
public:
  DropShadowAttribute(VRefCnt* cnt, Ref<ShapeAttribute> shapeAttr)
    : Attribute(cnt)
    , m_shapeAttr(shapeAttr)
  {
    observe(m_shapeAttr);
  }

  void  render(Renderer* renderer);
  Bound onRevalidate() override;

  VGG_ATTRIBUTE(DropShadowStyle, std::vector<DropShadow>, m_shadow);
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
  void  render(Renderer* renderer);
  Bound onRevalidate() override;
  VGG_ATTRIBUTE(InnerShadowStyle, std::vector<InnerShadow>, m_shadow);
  VGG_CLASS_MAKE(InnerShadowAttribute);

private:
  friend class RenderNode;
  Ref<ShapeAttribute>              m_shapeAttr;
  std::vector<InnerShadow>         m_shadow;
  std::optional<InnerShadowEffect> m_innerShadowEffects;
};

class ObjectAttribute : public Attribute // fill + border
{
public:
  ObjectAttribute(VRefCnt* cnt, Ref<RenderObjectAttribute> renderObject)
    : Attribute(cnt)
    //, m_shapeAttr(shape)
    , m_renderObjectAttr(renderObject)
  {
    // observe(m_shapeAttr);
    observe(m_renderObjectAttr);
  }

  bool hasFill() const
  {
    return m_hasFill;
  }

  sk_sp<SkImageFilter> asObjectMaskFilter()
  {
    return m_maskFilter;
  }

  sk_sp<SkImageFilter> imageFilter() const
  {
    return 0;
  }

  sk_sp<SkBlender> blender() const
  {
    return 0;
  }

  void  render(Renderer* renderer);
  Bound onRevalidate() override;

  VGG_ATTRIBUTE(FillStyle, std::vector<Fill>, m_fills);
  VGG_ATTRIBUTE(BorderStyle, std::vector<Border>, m_borders);
  VGG_ATTRIBUTE(RenderObject, Ref<RenderObjectAttribute>, m_renderObjectAttr);

  VGG_CLASS_MAKE(ObjectAttribute);

private:
  friend class RenderNode;
  std::pair<SkRect, std::optional<SkPaint>> revalidateObjectBounds(
    const std::vector<Border>& borders,
    const SkRect&              bounds);

  void revalidateMaskFilter(const SkPaint& paint, const SkRect& bounds);

  std::function<SkRect(Renderer* renderer)> m_onDrawFill;
  // Ref<ShapeAttribute>                       m_shapeAttr;

  Ref<RenderObjectAttribute> m_renderObjectAttr;

  sk_sp<SkImageFilter> m_maskFilter;
  std::vector<Fill>    m_fills;
  std::vector<Border>  m_borders;
  bool                 m_hasFill;
  // std::optional<ObjectShader> m_styleDisplayList; // fill + border
};

class StyleObjectAttribute : public Attribute
{
public:
  StyleObjectAttribute(
    VRefCnt*                     cnt,
    Ref<InnerShadowAttribute>    innerShadow,
    Ref<DropShadowAttribute>     dropShadow,
    Ref<ObjectAttribute>         object,
    Ref<BackgroundBlurAttribute> backgroundBlur)
    : Attribute(cnt)
    , m_innerShadowAttr(std::move(innerShadow))
    , m_dropShadowAttr(std::move(dropShadow))
    , m_objectAttr(std::move(object))
    , m_backgroundBlurAttr(std::move(backgroundBlur))
  {
    observe(m_innerShadowAttr);
    observe(m_dropShadowAttr);
    observe(m_objectAttr);
    observe(m_backgroundBlurAttr);
  }

  void render(Renderer* renderer);

  sk_sp<SkImageFilter> getBackgroundBlurImageFilter() const
  {
    return m_dropbackImageFilter;
  }

  Bound onRevalidate() override;

  VGG_CLASS_MAKE(StyleObjectAttribute);

private:
  friend class RenderNode;

  void revalidateDropbackFilter(const SkRect& bounds);

  Ref<InnerShadowAttribute>    m_innerShadowAttr;
  Ref<DropShadowAttribute>     m_dropShadowAttr;
  Ref<ObjectAttribute>         m_objectAttr;
  Ref<BackgroundBlurAttribute> m_backgroundBlurAttr;

  SkRect               m_objectBounds;
  sk_sp<SkImageFilter> m_bgBlurImageFilter;

  sk_sp<SkImageFilter> m_dropbackImageFilter;
  sk_sp<SkImageFilter> m_objectImageFilter;
};
} // namespace VGG::layer
