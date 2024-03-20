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
#include "ShapeAttribute.hpp"
#include "LayerAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "MaskAttribute.hpp"
#include "TransformAttribute.hpp"

namespace VGG::layer
{

class RenderNode : public VNode
{
public:
  RenderNode(
    VRefCnt*                  cnt,
    Ref<TransformAttribute>   transform,
    Ref<StyleObjectAttribute> styleObject,
    Ref<LayerFXAttribute>     layerPostProcess,
    Ref<AlphaMaskAttribute>   alphaMask,
    Ref<ShapeMaskAttribute>   shapeMask,
    Ref<ShapeAttribute>       shape)
    : VNode(cnt)
    , m_transformAttr(transform)
    , m_styleObjectAttr(styleObject)
    , m_alphaMaskAttr(alphaMask)
    , m_shapeMaskAttr(shapeMask)
    , m_shapeAttr(shape)
  {
    observe(m_transformAttr);
    observe(m_styleObjectAttr);
    observe(m_alphaMaskAttr);
    observe(m_shapeMaskAttr);
    observe(m_shapeAttr);
  }
  void  render(Renderer* renderer);
  Bound onRevalidate() override;
  VGG_CLASS_MAKE(RenderNode);

  class Accessor
  {
  public:
    Accessor(const Accessor&) = delete;
    Accessor& operator=(const Accessor&) = delete;
    Accessor(Accessor&&) = default;
    Accessor& operator=(Accessor&&) = default;
    ~Accessor() = default;

  private:
    Accessor() = default;
    Accessor(
      TransformAttribute*   transformAttr,
      ShapeAttribute*       shapeAttr,
      AlphaMaskAttribute*   alphaMaskAttr,
      ShapeMaskAttribute*   shapemaskAttr,
      DropShadowAttribute*  dropShadowAttr,
      InnerShadowAttribute* innerShadowAttr)
      : m_transformAttr(transformAttr)
      , m_alphaMaskAttr(alphaMaskAttr)
      , m_shapeMaskAttr(shapemaskAttr)
      , m_shapeAttr(shapeAttr)
      , m_dropShadowAttr(dropShadowAttr)
      , m_innerShadowAttr(innerShadowAttr)
    {
    }
    friend class RenderNode;
    TransformAttribute*   m_transformAttr;
    AlphaMaskAttribute*   m_alphaMaskAttr;
    ShapeMaskAttribute*   m_shapeMaskAttr;
    ShapeAttribute*       m_shapeAttr;
    DropShadowAttribute*  m_dropShadowAttr;
    InnerShadowAttribute* m_innerShadowAttr;
  };

  Accessor* accessor()
  {
    return &m_accessor;
  }

private:
  SkRect                              recorder(Renderer* renderer);
  std::pair<sk_sp<SkPicture>, SkRect> revalidatePicture(const SkRect& bounds);

  void beginLayer(
    Renderer*            renderer,
    const SkPaint*       paint,
    const VShape*        clipShape,
    sk_sp<SkImageFilter> backdropFilter);

  void                      endLayer(Renderer* renderer);
  Ref<TransformAttribute>   m_transformAttr;
  Ref<StyleObjectAttribute> m_styleObjectAttr;
  Ref<AlphaMaskAttribute>   m_alphaMaskAttr;
  Ref<ShapeMaskAttribute>   m_shapeMaskAttr;
  Ref<ShapeAttribute>       m_shapeAttr;
  sk_sp<SkPicture>          m_picture;

  Accessor m_accessor;
};

Ref<RenderNode> renderResult(
  PaintNode*                   paintNode,
  Ref<TransformAttribute>      transform,
  Ref<AlphaMaskAttribute>      alphaMask,
  Ref<BackgroundBlurAttribute> backgroundBlur,
  Ref<ShapeMaskAttribute>      shapeMask,
  Ref<InnerShadowAttribute>    innerShadow,
  Ref<DropShadowAttribute>     dropShadow,
  Ref<ObjectAttribute>         object,
  Ref<ShapeAttribute>          shape)
{
  auto style = Ref<StyleObjectAttribute>(
    V_NEW<StyleObjectAttribute>(innerShadow, dropShadow, object, backgroundBlur));
  auto alphaMaskAttribute = AlphaMaskAttribute::Make(paintNode, nullptr);
  auto layerPostProcess = Ref<LayerFXAttribute>(V_NEW<LayerFXAttribute>(style));
  alphaMaskAttribute->setInputImageFilter(layerPostProcess);
  auto result =
    RenderNode::Make(transform, style, layerPostProcess, alphaMaskAttribute, shapeMask, shape);
  return result;
};
} // namespace VGG::layer
