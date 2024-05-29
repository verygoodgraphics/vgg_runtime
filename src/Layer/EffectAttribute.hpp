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
#include "ImageFilterAttribute.hpp"

#include "Layer/Core/Attrs.hpp"

namespace VGG::layer
{

class StyleItem;

class StyleAttribute;

class BackdropFXAttribute : public ImageFilterAttribute
{
public:
  BackdropFXAttribute(VRefCnt* cnt)
    : ImageFilterAttribute(cnt)
  {
  }
  VGG_ATTRIBUTE(BackgroundBlur, const std::vector<BackgroundFX>&, m_blurs);
  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_imageFilter;
  }
  Bounds onRevalidate(Revalidation* inv, const glm::mat3 & mat) override;
  VGG_CLASS_MAKE(BackdropFXAttribute);

private:
  friend class RenderNode;
  std::vector<BackgroundFX> m_blurs;
  sk_sp<SkImageFilter>      m_imageFilter;
};

class LayerFXAttribute : public ImageFilterAttribute
{
public:
  LayerFXAttribute(VRefCnt* cnt, WeakRef<StyleItem> styleItem);
  VGG_ATTRIBUTE(LayerBlur, const std::vector<LayerFX>&, m_blurs);
  Bounds               onRevalidate(Revalidation* inv, const glm::mat3 & mat) override;
  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_imageFilter;
  }
  VGG_CLASS_MAKE(LayerFXAttribute);

private:
  friend class RenderNode;
  SkRect revalidateLayerImageFilter(const SkRect& bounds);

  WeakRef<StyleItem>   m_styleItem;
  std::vector<LayerFX> m_blurs;
  sk_sp<SkImageFilter> m_imageFilter;
};

} // namespace VGG::layer
