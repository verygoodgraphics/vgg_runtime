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
#include "MaskAttribute.hpp"
// #include "ObjectAttribute.hpp"

namespace VGG::layer
{

class LayerPreProcessAttribute;
class LayerPostProcessAttribute;

class StyleObjectAttribute;

class LayerBlurAttribute : public ImageFilterAttribute
{
public:
  LayerBlurAttribute(VRefCnt* cnt)
    : ImageFilterAttribute(cnt)
  {
  }
  VGG_CLASS_MAKE(LayerBlurAttribute);

private:
};

class BackgroundBlurAttribute : public Attribute
{
public:
  BackgroundBlurAttribute(VRefCnt* cnt)
    : Attribute(cnt)
  {
  }
  sk_sp<SkImageFilter> getImageFilter() const
  {
    return m_filter;
  }
  VGG_CLASS_MAKE(BackgroundBlurAttribute);

private:
  sk_sp<SkImageFilter> m_filter;
};

class LayerPostProcessAttribute : public ImageFilterAttribute
{
public:
  LayerPostProcessAttribute(
    VRefCnt*                  cnt,
    Ref<AlphaMaskAttribute>   alphaMask,
    Ref<LayerBlurAttribute>   layerBlur,
    Ref<StyleObjectAttribute> styleObjectAttr)
    : ImageFilterAttribute(cnt)
    , m_alphaMaskAttr(alphaMask)
    , m_layerBlurAttr(layerBlur)
    , m_styleObjectAttr(styleObjectAttr)
  {
    observe(m_alphaMaskAttr);
    observe(m_layerBlurAttr);
    observe(m_styleObjectAttr);
  }

  Bound onRevalidate() override
  {
    // return union bound including dropshadow and object bound;

    m_alphaMaskAttr->revalidate();
    // m_styleObjectAttr->revalidate();
    auto skRect = toSkRect(m_styleObjectAttr->bound());
    m_imageFilter->computeFastBounds(skRect, &skRect);
    //_->ensureDropShadowEffects(_->style.dropShadow, path);
    return Bound();
    // auto alphaMaskFilter = m_alphaMaskAttr->getImageFilter();
    // auto layerBlurFilter = m_layerBlurAttr->getImageFilter();
    // auto styleObjectFilter = m_styleObjectAttr->getImageFilter();
    // m_imageFilter = SkImageFilters::Compose(
    //   alphaMaskFilter, layerBlurFilter, styleObjectFilter);
  }

  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_imageFilter;
  }

  VGG_CLASS_MAKE(LayerPostProcessAttribute);

private:
  Ref<AlphaMaskAttribute>   m_alphaMaskAttr;
  Ref<LayerBlurAttribute>   m_layerBlurAttr;
  Ref<StyleObjectAttribute> m_styleObjectAttr;
  sk_sp<SkImageFilter>      m_imageFilter;
};

} // namespace VGG::layer
