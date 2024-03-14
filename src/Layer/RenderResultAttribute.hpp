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
#include "LayerAttribute.hpp"
#include "ObjectAttribute.hpp"

namespace VGG::layer
{

class RenderResultAttribute : public Attribute
{
public:
  RenderResultAttribute(
    VRefCnt*                  cnt,
    Ref<TransformAttribute>   transform,
    Ref<LayerAttribute>       layerAttr,
    Ref<StyleObjectAttribute> styleObject,
    Ref<ShapeMaskAttribute>   shapeMask,
    Ref<ShapeAttribute>       primitive)
    : Attribute(cnt)
    , m_transformAttr(transform)
    , m_layerAttr(layerAttr)
    , m_styleObjectAttr(styleObject)
    , m_shapeMaskAttr(shapeMask)
    , m_shapeAttr(primitive)
  {
    observe(m_transformAttr);
    observe(m_styleObjectAttr);
    observe(m_shapeMaskAttr);
    observe(m_shapeAttr);
  }

  void  render(Renderer* renderer);
  Bound onRevalidate() override;
  VGG_CLASS_MAKE(RenderResultAttribute);

private:
  void draw(Renderer* renderer);
  void beginLayer(
    Renderer*            renderer,
    const SkPaint*       paint,
    const VShape*        clipShape,
    sk_sp<SkImageFilter> backdropFilter);

  void                      endLayer(Renderer* renderer);
  Ref<TransformAttribute>   m_transformAttr;
  Ref<LayerAttribute>       m_layerAttr;
  Ref<StyleObjectAttribute> m_styleObjectAttr;
  Ref<ShapeMaskAttribute>   m_shapeMaskAttr;
  Ref<ShapeAttribute>       m_shapeAttr;
  sk_sp<SkPicture>          m_picture;
};

Ref<RenderResultAttribute> renderResultDAG(
  Ref<TransformAttribute>      transform,
  Ref<AlphaMaskAttribute>      alphaMask,
  Ref<LayerBlurAttribute>      layerBlur,
  Ref<BackgroundBlurAttribute> backgroundBlur,
  Ref<ShapeMaskAttribute>      shapeMask,
  Ref<InnerShadowAttribute>    innerShadow,
  Ref<DropShadowAttribute>     dropShadow,
  Ref<ObjectAttribute>         object,
  Ref<ShapeAttribute>          primitive)
{
  auto layerProcess = V_NEW<LayerPreProcessAttribute>(backgroundBlur);
  auto layerPostProcess = V_NEW<LayerPostProcessAttribute>(alphaMask, layerBlur);
  auto layer = V_NEW<LayerAttribute>(layerProcess, layerPostProcess);
  auto style = V_NEW<StyleObjectAttribute>(innerShadow, dropShadow, object);
  auto result = RenderResultAttribute::Make(transform, layer, style, shapeMask);
  return result;
};
} // namespace VGG::layer
