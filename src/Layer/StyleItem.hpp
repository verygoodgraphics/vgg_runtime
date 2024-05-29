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
#include "Layer/Core/RenderNode.hpp"
#include "Layer/Effects.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include "AttributeNode.hpp"
#include "ShapeAttribute.hpp"
#include "EffectAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "MaskAttribute.hpp"
#include "TransformAttribute.hpp"

#include "Layer/Core/VNode.hpp"
#include "Utility/HelperMacro.hpp"

namespace VGG::layer
{

class StyleItem__pImpl;
class StyleItem : public RenderNode
{
  VGG_DECL_IMPL(StyleItem);

public:
  using Creator = std::function<Ref<GraphicItem>(VAllocator* alloc, StyleItem*)>;
  StyleItem(VRefCnt* cnt, PaintNode* node, Ref<TransformAttribute> transform, Creator creator);

  void render(Renderer* renderer) override;

  void nodeAt(int x, int y, NodeVisitor vistor, void* userData) override
  {
  }

#ifdef VGG_LAYER_DEBUG
  virtual void debug(Renderer* render) override;
#endif

  void renderAsMask(Renderer* render);

  Bounds effectBounds() const override;

  Bounds objectBounds();

  // Model attributes, these should be moved out of StyleItem. Maybe PaintNode is a good choice
  void applyFillStyle(const std::vector<Fill>& fills)
  {
    ASSERT(m_fillEffect);
    if (m_fills == fills)
      return;
    m_fills = fills;
    m_fillEffect->applyFillStyle(std::move(fills));
  }

  void applyBorderStyle(const std::vector<Border>& borders)
  {
    ASSERT(m_borderEffect);
    if (m_borders == borders)
      return;
    m_borders = borders;
    m_borderEffect->applyBorderStyle(std::move(borders));
  }

  const std::vector<Fill>& getModelFillStyle() const
  {
    ASSERT(m_fillEffect);
    return m_fills;
  }

  const std::vector<Border>& getModelBorderStyle() const
  {
    ASSERT(m_borderEffect);
    return m_borders;
  }

  // style manipulation APIs (view attributes)

  const std::vector<Ref<Brush>>& getBrush() const
  {
    ASSERT(m_fillEffect);
    return m_fillEffect->fills();
  }

  const std::vector<Ref<BorderBrush>>& getBorderBrush() const
  {
    ASSERT(m_borderEffect);
    return m_borderEffect->borders();
  }

  InnerShadowAttribute* getInnerShadowAttribute() const
  {
    return m_innerShadowAttr.get();
  }

  bool isInvalid() const
  {
    return VNode::isInvalid();
  }

  const Bounds& styleEffectBounds() const
  {
    return m_styleEffectBounds;
  }

  static std::pair<Ref<StyleItem>, std::unique_ptr<Accessor>> MakeRenderNode( // NOLINT
    VAllocator*             alloc,
    PaintNode*              node,
    Ref<TransformAttribute> transform,
    Creator                 creator);

  ~StyleItem();

protected:
  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override;

private:
  VGG_CLASS_MAKE(StyleItem);

  void                                recorder(Renderer* renderer);
  std::pair<sk_sp<SkPicture>, SkRect> revalidatePicture(const SkRect& bounds);

  bool hasNewLayer() const;

  void revalidateEffectsBounds();

  struct AutoLayerRestore
  {
    template<typename F>
    AutoLayerRestore(Renderer* renderer, bool doSave, F&& f)
      : m_renderer(renderer)
    {
      ASSERT(m_renderer);
      m_saveCount = m_renderer->canvas()->getSaveCount();
      if (doSave)
      {
        m_renderer->canvas()->save();
        SkPaint              paint;
        VShape               shape;
        sk_sp<SkImageFilter> backdropFilter;
        f(paint, shape, backdropFilter);
        if (!shape.isEmpty())
        {
          shape.clip(m_renderer->canvas(), SkClipOp::kIntersect);
        }
        auto layerBounds = shape.bounds();
        m_renderer->canvas()->saveLayer(
          SkCanvas::SaveLayerRec(&layerBounds, &paint, backdropFilter.get(), 0));
      }
    }

    ~AutoLayerRestore()
    {
      m_renderer->canvas()->restoreToCount(m_saveCount);
      m_renderer = nullptr;
    }

  private:
    Renderer* m_renderer{ nullptr };
    int       m_saveCount;
  };

  Ref<TransformAttribute> m_transformAttr; // Removed from StyleItem

  ////////////////////// StyleAttribute begin
  Ref<InnerShadowAttribute> m_innerShadowAttr;
  Ref<DropShadowAttribute>  m_dropShadowAttr;

  ////////////////////// ObjectAttribute begin

  // Ref<ObjectAttribute> m_objectAttr;

  Ref<GraphicItem>    m_graphicItem;
  std::vector<Fill>   m_fills;
  std::vector<Border> m_borders;
  bool                m_hasFill{ false };

  Bounds onRevalidateObject();

  ////////////////////// ObjectAttribute end
  Ref<BackdropFXAttribute> m_backgroundBlurAttr;

  SkRect m_objectEffectBounds;
  Bounds m_styleEffectBounds;

  sk_sp<SkImageFilter> m_bgBlurImageFilter;

  sk_sp<SkImageFilter>        m_dropbackImageFilter;
  const sk_sp<SkImageFilter>& backdropImageFilter() const
  {
    return m_dropbackImageFilter;
  }
  sk_sp<SkImageFilter> m_objectImageFilter;

  void observeStyleAttribute();

  void unobserveStyleAttribute();

  void revalidateDropbackFilter(const SkRect& bounds);

  void onRenderStyle(Renderer* renderer);

  Bounds onRevalidateStyle();

  ////////////////////// StyleAttribute end

  Ref<AlphaMaskAttribute> m_alphaMaskAttr;
  Ref<ShapeMaskAttribute> m_shapeMaskAttr;
  Bounds                  m_effectsBounds;
  sk_sp<SkPicture>        m_picture;

  Ref<StackFillEffectImpl>   m_fillEffect;
  Ref<StackBorderEffectImpl> m_borderEffect;
};
} // namespace VGG::layer
