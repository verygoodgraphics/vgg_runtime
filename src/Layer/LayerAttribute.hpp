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

namespace VGG::layer
{

class LayerPreProcessAttribute;
class LayerPostProcessAttribute;

class LayerEffectsAttribute : public Attribute
{
public:
  LayerEffectsAttribute(VRefCnt* cnt)
    : Attribute(cnt)
  {
  }
  virtual sk_sp<SkImageFilter> getImageFilter() const = 0;
  VGG_CLASS_MAKE(LayerEffectsAttribute);

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

class LayerPreProcessAttribute : public LayerEffectsAttribute
{
public:
  LayerPreProcessAttribute(
    VRefCnt*                     cnt,
    LayerAttribute*              layerAttr,
    Ref<BackgroundBlurAttribute> backgroundBlurAttribute,
    Ref<ShapeAttribute>          primitive)
    : LayerEffectsAttribute(cnt)
    , m_backgroundBlurAttribute(backgroundBlurAttribute)
    , m_primitiveAttr(primitive)
    , m_layerAttri(layerAttr)
  {
    observe(m_backgroundBlurAttribute);
  }
  VGG_ATTRIBUTE(BackgroundBlur, Ref<BackgroundBlurAttribute>, m_backgroundBlurAttribute);
  VGG_ATTRIBUTE(ShapeAttribute, Ref<ShapeAttribute>, m_primitiveAttr);
  sk_sp<SkImageFilter> getImageFilter() const override
  {
    return m_backgroundBlurAttribute->getImageFilter();
  }
  Bound onRevalidate() override;

private:
  Ref<BackgroundBlurAttribute> m_backgroundBlurAttribute;
  Ref<ShapeAttribute>          m_primitiveAttr;
  LayerAttribute*              m_layerAttri;
  sk_sp<SkImageFilter>         m_imageFilter;
};

class LayerPostProcessAttribute : public LayerEffectsAttribute
{
public:
  LayerPostProcessAttribute(
    VRefCnt*                cnt,
    Ref<AlphaMaskAttribute> alphaMask,
    Ref<LayerBlurAttribute> layerBlur,
    Ref<ShapeAttribute>     primitive)
    : LayerEffectsAttribute(cnt)
    , m_alphaMaskAttr(alphaMask)
    , m_layerBlurAttr(layerBlur)
    , m_primitiveAttr(primitive)
  {
    observe(m_alphaMaskAttr);
    observe(m_layerBlurAttr);
    observe(m_primitiveAttr);
  }

  VGG_CLASS_MAKE(LayerPostProcessAttribute);

private:
  Ref<AlphaMaskAttribute> m_alphaMaskAttr;
  Ref<LayerBlurAttribute> m_layerBlurAttr;
  Ref<ShapeAttribute>     m_primitiveAttr;
  sk_sp<SkImageFilter>    m_imageFilter;
};

class LayerAttribute : public Attribute
{
public:
  LayerAttribute(
    VRefCnt*                       cnt,
    Ref<LayerPreProcessAttribute>  preLayer,
    Ref<LayerPostProcessAttribute> postLayer)
    : Attribute(cnt)
  {
  }
  VGG_ATTRIBUTE(PreProcess, Ref<LayerPreProcessAttribute>, m_preLayer);
  VGG_ATTRIBUTE(PostProcess, Ref<LayerPostProcessAttribute>, m_postLayer);
  VGG_CLASS_MAKE(LayerAttribute);
  Bound onRevalidate() override
  {
    auto a = m_preLayer->revalidate();
    auto b = m_postLayer->revalidate();
    a.unionWith(b);
    return a;
  }

private:
  Ref<LayerPreProcessAttribute>  m_preLayer;
  Ref<LayerPostProcessAttribute> m_postLayer;
};

} // namespace VGG::layer
