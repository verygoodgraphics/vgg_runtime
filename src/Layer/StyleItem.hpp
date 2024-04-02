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
#include "RenderNode.hpp"
#include "AttributeNode.hpp"
#include "ShapeAttribute.hpp"
#include "EffectAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "MaskAttribute.hpp"
#include "TransformAttribute.hpp"

#include "Layer/Core/VNode.hpp"

namespace VGG::layer
{

class StyleItem : public GraphicItem
{
public:
  StyleItem(
    VRefCnt*                cnt,
    Ref<TransformAttribute> transform,
    Ref<StyleAttribute>     styleObject,
    Ref<LayerFXAttribute>   layerPostProcess,
    Ref<AlphaMaskAttribute> alphaMask,
    Ref<ShapeMaskAttribute> shapeMask,
    Ref<ShapeAttribute>     shape)
    : GraphicItem(cnt)
    , m_transformAttr(transform)
    , m_objectAttr(styleObject)
    , m_alphaMaskAttr(alphaMask)
    , m_shapeMaskAttr(shapeMask)
  {
    observe(m_transformAttr);
    observe(m_objectAttr);
    observe(m_alphaMaskAttr);
    observe(m_shapeMaskAttr);
  }
  void render(Renderer* renderer) override;

  void renderAsMask(Renderer* render);

  ShapeAttribute* shape() const override
  {
    return 0;
  }

  sk_sp<SkImageFilter> getMaskFilter() const override
  {
    return 0;
  }

  Bounds effectBounds() const override;

  bool isInvalid() const
  {
    return VNode::isInvalid();
  }
  Bounds onRevalidate() override;

  using Creator = std::function<Ref<GraphicItem>(VAllocator* alloc, ObjectAttribute*)>;
  static std::pair<Ref<StyleItem>, std::unique_ptr<Accessor>> MakeRenderNode( // NOLINT
    VAllocator*             alloc,
    PaintNode*              node,
    Ref<TransformAttribute> transform,
    Creator                 creator);

  ~StyleItem();

private:
  VGG_CLASS_MAKE(StyleItem);

  SkRect                              recorder(Renderer* renderer);
  std::pair<sk_sp<SkPicture>, SkRect> revalidatePicture(const SkRect& bounds);

  void beginLayer(
    Renderer*            renderer,
    const SkPaint*       paint,
    const VShape*        clipShape,
    sk_sp<SkImageFilter> backdropFilter);

  void                    endLayer(Renderer* renderer);
  Ref<TransformAttribute> m_transformAttr;
  Ref<StyleAttribute>     m_objectAttr;
  Ref<AlphaMaskAttribute> m_alphaMaskAttr;
  Ref<ShapeMaskAttribute> m_shapeMaskAttr;
  Bounds                  m_effectsBounds;
  sk_sp<SkPicture>        m_picture;
};
} // namespace VGG::layer
