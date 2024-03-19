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
#include "AttributeGraph.hpp"
#include "ImageFilterAttribute.hpp"
#include "LayerAttribute.hpp"

namespace VGG::layer
{
class ObjectAttribute : public Attribute // fill + border
{
public:
  ObjectAttribute(VRefCnt* cnt, Ref<ShapeAttribute> primitive)
    : Attribute(cnt)
    , m_shapeAttr(primitive)
  {
  }

  bool hasFill() const
  {
    return m_hasFill;
  }

  Bound onRevalidate() override
  {
    for (const auto& f : m_fills)
    {
      if (f.isEnabled)
      {
        m_hasFill = true;
        break;
      }
    }

    // ideally, we need accurate bound of fill + border
    return Bound();
  }

  VGG_ATTRIBUTE(Shape, Ref<ShapeAttribute>, m_shapeAttr);
  VGG_ATTRIBUTE(FillStyle, std::vector<Fill>, m_fills);
  VGG_ATTRIBUTE(BorderStyle, std::vector<Border>, m_borders);

private:
  Ref<ShapeAttribute> m_shapeAttr;
  std::vector<Fill>   m_fills;
  std::vector<Border> m_borders;
  bool                m_hasFill;
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
    , m_innerShadowAttr(innerShadow)
    , m_dropShadowAttr(dropShadow)
    , m_objectAttr(object)
    , m_backgroundBlurAttr(backgroundBlur)
  {
    observe(m_innerShadowAttr);
    observe(m_dropShadowAttr);
    observe(m_objectAttr);
    observe(m_backgroundBlurAttr);
  }

  void render(Renderer* renderer) override;

  sk_sp<SkImageFilter> getBackgroundBlurImageFilter() const
  {
    return m_backgroundBlurAttr->getImageFilter();
  }

  sk_sp<SkImageFilter> asImageFilter()
  {
    return m_objectImageFilter;
  }

  Bound onRevalidate() override
  {
    auto b = m_objectAttr->revalidate();
    return b;
  }

  VGG_CLASS_MAKE(StyleObjectAttribute);

private:
  Ref<InnerShadowAttribute>    m_innerShadowAttr;
  Ref<DropShadowAttribute>     m_dropShadowAttr;
  Ref<ObjectAttribute>         m_objectAttr;
  Ref<BackgroundBlurAttribute> m_backgroundBlurAttr;

  sk_sp<SkImageFilter> m_objectImageFilter;
};
} // namespace VGG::layer
