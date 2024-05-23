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
    Ref<ShapeAttribute>     shape);
  void render(Renderer* renderer) override;

#ifdef VGG_LAYER_DEBUG
  virtual void debug(Renderer* render) override;
#endif

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

  void setFillStyle(std::vector<Fill> fills)
  {
    ASSERT(m_fillEffect);
    m_fillEffect->setFillStyle(std::move(fills));
  }
  void setBorderStyle(std::vector<Border> borders)
  {
    ASSERT(m_borderEffect);
    m_borderEffect->setBorderStyle(std::move(borders));
  }

  const std::vector<Ref<FillPenNode>>& getFillStyle() const
  {
    ASSERT(m_fillEffect);
    return m_fillEffect->fills();
  }
  const std::vector<Ref<BorderPenNode>>& getBorderStyle() const
  {
    ASSERT(m_borderEffect);
    return m_borderEffect->borders();
  }

  bool isInvalid() const
  {
    return VNode::isInvalid();
  }

  using Creator = std::function<Ref<GraphicItem>(VAllocator* alloc, ObjectAttribute*)>;
  static std::pair<Ref<StyleItem>, std::unique_ptr<Accessor>> MakeRenderNode( // NOLINT
    VAllocator*             alloc,
    PaintNode*              node,
    Ref<TransformAttribute> transform,
    Creator                 creator);

  ~StyleItem();

protected:
  Bounds onRevalidate() override;

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

  Ref<TransformAttribute> m_transformAttr;
  Ref<StyleAttribute>     m_styleAttr;

  Ref<AlphaMaskAttribute> m_alphaMaskAttr;
  Ref<ShapeMaskAttribute> m_shapeMaskAttr;
  Bounds                  m_effectsBounds;
  sk_sp<SkPicture>        m_picture;

  Ref<StackFillEffectImpl>   m_fillEffect;
  Ref<StackBorderEffectImpl> m_borderEffect;
};
} // namespace VGG::layer
