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

#include "PenNode.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VUtils.hpp"

#include "Layer/LRUCache.hpp"
#include "Layer/LayerCache.h"
#include "Layer/SkSL.hpp"
#include "Layer/Core/EffectNode.hpp"
#include "Layer/GraphicItem.hpp"
#include "Layer/VSkia.hpp"

#include <core/SkBlender.h>
#include <include/effects/SkRuntimeEffect.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/core/SkSamplingOptions.h>

namespace VGG::layer
{
class Renderer;
class VShape;

class Effects
{
public:
  virtual void render(Renderer* render, const VShape& shape) = 0;
  virtual ~Effects() = default;
};

sk_sp<SkColorFilter> makeColorFilter(const ImageFilter& imageFilter);
sk_sp<SkShader>      makeFitPattern(const Bounds& bounds, const PatternFit& p);
sk_sp<SkShader>      makeFillPattern(const Bounds& bounds, const PatternFill& p);
sk_sp<SkShader>      makeStretchPattern(const Bounds& bounds, const PatternStretch& p);
sk_sp<SkShader>      makeTilePattern(const Bounds& bounds, const PatternTile& p);
sk_sp<SkShader>      makeGradientLinear(const Bounds& bounds, const GradientLinear& g);

sk_sp<SkImageFilter>   makeMotionBlurFilter(const MotionBlur& blur);
sk_sp<SkImageFilter>   makeRadialBlurFilter(const RadialBlur& blur, const Bounds& bounds);
sk_sp<SkImageFilter>   makeLayerBlurFilter(const GaussianBlur& blur);
sk_sp<SkImageFilter>   makeBackgroundBlurFilter(const GaussianBlur& blur);
sk_sp<SkRuntimeEffect> getOrCreateEffect(EffectCacheKey key, const char* sksl);
sk_sp<SkBlender>       getOrCreateBlender(EffectCacheKey name, const char* sksl);

inline sk_sp<SkBlender> getMaskBlender(EAlphaMaskType type)
{
  switch (type)
  {
    case AM_ALPHA:
      return getOrCreateBlender("alpha", g_alphaMaskBlender);
    case AM_LUMINOSITY:
      return getOrCreateBlender("lumi", g_luminosityBlender);
    case AM_INVERSE_LUMINOSITY:
      return getOrCreateBlender("invLumi", g_invLuminosityBlender);
  }
  DEBUG("No corresponding mask blender");
  return nullptr;
}

inline sk_sp<SkShader> makePatternShader(const Bounds& bounds, const Pattern& pattern)
{
  sk_sp<SkShader> shader;
  std::visit(
    Overloaded{ [&](const PatternFill& p) { shader = makeFillPattern(bounds, p); },
                [&](const PatternFit& p) { shader = makeFitPattern(bounds, p); },
                [&](const PatternStretch& p) { shader = makeStretchPattern(bounds, p); },
                [&](const PatternTile& p) { shader = makeTilePattern(bounds, p); } },
    pattern.instance);
  return shader;
}

inline SkMatrix makeMatrix(
  const Bounds&                         bounds,
  const glm::vec2&                      f,
  const glm::vec2&                      t,
  const std::variant<float, glm::vec2>& ellipse)
{
  const auto from = bounds.map(bounds.size() * f);
  const auto to = bounds.map(bounds.size() * t);

  auto theta = [](const glm::vec2& from, const glm::vec2& to)
  {
    const auto d = to - from;
    return std::atan2(d.y, d.x);
  };

  auto ratio = 1.f;
  std::visit(
    Overloaded{ [&](const float& f) { ratio = f; },
                [&](const glm::vec2& p)
                {
                  auto       pp = bounds.map(bounds.size() * p);
                  const auto a = glm::distance(from, to);
                  const auto b = glm::distance(from, pp);
                  ratio = (a == 0.f) ? 1.f : b / a;
                } },
    ellipse);
  SkMatrix mat = SkMatrix::I();
  mat.postTranslate(-from.x, -from.y);
  mat.postScale(1.0, ratio);
  mat.postRotate(rad2deg(theta(from, to)));
  mat.postTranslate(from.x, from.y);
  return mat;
}
template<typename G>
inline sk_sp<SkShader> makeGradientRadial(const Bounds& bounds, const G& g)
{
  if (g.stops.empty())
    return nullptr;

  auto       f = bounds.map(bounds.size() * g.from);
  auto       t = bounds.map(bounds.size() * g.to);
  SkScalar   r = glm::distance(f, t);
  const auto minPosition = 0.f;
  const auto maxPosition = 1.f;

  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;
  for (auto it = g.stops.begin(); it != g.stops.end(); ++it)
  {
    colors.push_back(it->color);
    auto p = it->position;
    positions.push_back((p - minPosition) / (maxPosition - minPosition));
  }
  auto mat = makeMatrix(bounds, g.from, g.to, g.ellipse);
  return SkGradientShader::MakeRadial(
    { f.x, f.y },
    r,
    colors.data(),
    positions.data(),
    colors.size(),
    SkTileMode::kClamp,
    0,
    &mat);
}

template<typename G>
inline sk_sp<SkShader> makeGradientAngular(const Bounds& bounds, const G& g)
{
  if (g.stops.empty())
    return nullptr;

  const auto            minPosition = g.stops.front().position;
  const auto            maxPosition = g.stops.back().position;
  auto                  minPosColor = g.stops.front().color;
  auto                  maxPosColor = g.stops.back().color;
  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;
  size_t                sz = g.stops.size();

  if (minPosition > 0)
  {
    auto c = lerp(minPosColor, maxPosColor, (float)minPosition / (minPosition + 1 - maxPosition));
    colors.push_back(c);
    positions.push_back(0);
    sz += 1;
  }
  for (auto iter = g.stops.begin(); iter != g.stops.end(); ++iter)
  {
    colors.push_back(iter->color);
    positions.push_back(iter->position);
  }
  if (maxPosition < 1)
  {
    auto c = lerp(minPosColor, maxPosColor, (float)minPosition / (minPosition + 1 - maxPosition));
    colors.push_back(c);
    positions.push_back(1);
    sz += 1;
  }

  const auto mat = makeMatrix(bounds, g.from, g.to, g.ellipse);
  const auto from = bounds.map(bounds.size() * g.from);
  return SkGradientShader::MakeSweep(from.x, from.y, colors.data(), positions.data(), sz, 0, &mat);
}

template<typename G>
inline sk_sp<SkShader> makeGradientDiamond(const Bounds& bounds, const G& g)
{
  if (g.stops.empty())
    return nullptr;

  auto       f = bounds.map(bounds.size() * g.from);
  auto       t = bounds.map(bounds.size() * g.to);
  SkScalar   r = glm::distance(f, t);
  const auto minPosition = 0.f;
  const auto maxPosition = 1.f;

  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;
  for (auto it = g.stops.begin(); it != g.stops.end(); ++it)
  {
    colors.push_back(it->color);
    auto p = it->position;
    positions.push_back((p - minPosition) / (maxPosition - minPosition));
  }
  auto mat = makeMatrix(bounds, g.from, g.to, g.ellipse);
  mat.preTranslate(f.x, f.y);
  SkPoint pts[2] = { { 0, 0 }, { r / 2, r / 2 } };
  auto    linearShader = SkGradientShader::MakeLinear(
    pts,
    colors.data(),
    positions.data(),
    colors.size(),
    SkTileMode::kClamp,
    0,
    0);

  auto result = SkRuntimeEffect::MakeForShader(SkString(R"(
    uniform shader linearShader;
    vec4 main(vec2 inCoords) {
        return linearShader.eval(vec2(abs(inCoords.y) + abs(inCoords.x), 1.0));
    }
  )"));
  if (!result.effect)
  {
    DEBUG("Runtime Effect Failed[%s]: %s", "diamond", result.errorText.data());
    return nullptr;
  }
  sk_sp<SkShader> child[1] = { linearShader };
  auto            s = result.effect->makeShader(nullptr, child, 1, &mat);
  return s;
}

inline sk_sp<SkShader> makeGradientShader(const Bounds& bounds, const Gradient& gradient)
{
  sk_sp<SkShader> shader;
  std::visit(
    Overloaded{
      [&](const GradientLinear& p) { shader = makeGradientLinear(bounds, p); },
      [&](const GradientRadial& p) { shader = makeGradientRadial(bounds, p); },
      [&](const GradientAngular& p) { shader = makeGradientAngular(bounds, p); },
      [&](const GradientDiamond& p) { shader = makeGradientDiamond(bounds, p); },
    },
    gradient.instance);
  return shader;
}

inline void populateSkPaint(
  const FillType&       fillType,
  const ContextSetting& st,
  const SkRect&         rect,
  SkPaint&              paint)
{
  Bounds bounds{ rect.x(), rect.y(), rect.width(), rect.height() };
  std::visit(
    Overloaded{ [&](const Gradient& g)
                {
                  paint.setShader(makeGradientShader(bounds, g));
                  paint.setAlphaf(st.opacity);
                },
                [&](const Color& c)
                {
                  paint.setColor(c);
                  paint.setAlphaf(c.a * st.opacity);
                },
                [&](const Pattern& p)
                {
                  paint.setShader(makePatternShader(bounds, p));
                  paint.setAlphaf(st.opacity);
                } },
    fillType);
  auto bm = toSkBlendMode(st.blendMode);
  if (bm)
  {
    std::visit(
      Overloaded{ [&](const sk_sp<SkBlender>& blender) { paint.setBlender(blender); },
                  [&](const SkBlendMode& mode) { paint.setBlendMode(mode); } },
      *bm);
  }
}

inline void populateSkPaint(const Border& border, const SkRect& bounds, SkPaint& paint)
{
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setPathEffect(SkDashPathEffect::Make(
    border.dashedPattern.data(),
    border.dashedPattern.size(),
    border.dashedOffset));
  paint.setStrokeJoin(toSkPaintJoin(border.lineJoinStyle));
  paint.setStrokeCap(toSkPaintCap(border.lineCapStyle));
  paint.setStrokeMiter(border.miterLimit);
  paint.setStrokeWidth(border.thickness);
  populateSkPaint(border.type, border.contextSettings, bounds, paint);
}

void                     setGlobalSamplingOptions(const SkSamplingOptions& opt);
const SkSamplingOptions& getGlobalSamplingOptions();

sk_sp<SkImageFilter> makeInnerShadowImageFilter(
  const InnerShadow&   shadow,
  const Bounds&        bounds,
  bool                 shadowOnly,
  bool                 overrideSpread,
  sk_sp<SkImageFilter> input);

sk_sp<SkImageFilter> makeDropShadowImageFilter(
  const DropShadow&    shadow,
  const Bounds&        bounds,
  bool                 overrideSpread,
  sk_sp<SkImageFilter> input);

SkRect drawBorder(
  Renderer*                  renderer,
  const VShape&              border,
  const SkRect&              bounds,
  const std::vector<Border>& borders,
  sk_sp<SkBlender>           blender);

class GraphicItemEffectNode : public EffectNode
{
public:
  GraphicItemEffectNode(VRefCnt* cnt, Ref<GraphicItem> child)
    : EffectNode(cnt, std::move(child))
  {
    ASSERT(getChild());
  };

  void nodeAt(int x, int y, NodeVisitor vistor, void* userData) override
  {
  }

  Bounds effectBounds() const override
  {
    ASSERT(getChild());
    return getChild()->bounds();
  }

  void render(Renderer* renderer) override;

protected:
  ShapeAttribute* shape() const
  {
    return static_cast<GraphicItem*>(getChild().get())->shape();
  }

  virtual void onRenderShape(Renderer* renderer, const VShape& shape) = 0;

  Bounds onRevalidate() override
  {
    ASSERT(getChild());
    return getChild()->revalidate();
  }

private:
};

// implementation of Effect Nodes
class StackFillEffectImpl : public GraphicItemEffectNode
{
public:
  StackFillEffectImpl(VRefCnt* cnt, Ref<GraphicItem> child)
    : GraphicItemEffectNode(cnt, child)
  {
  }
  ~StackFillEffectImpl()
  {
    for (auto& pen : m_pens)
    {
      unobserve(pen);
    }
  }

  const std::vector<Ref<FillPenNode>>& fills() const
  {
    return m_pens;
  }

  FillPenNode* fill(int i) const
  {
    if (i >= (int)m_pens.size() || i < 0)
      return nullptr;
    return m_pens[i].get();
  }

  void setFillStyle(std::vector<Fill> fills);

  VGG_CLASS_MAKE(StackFillEffectImpl);

protected:
  void   onRenderShape(Renderer* renderer, const VShape& shape) override;
  Bounds onRevalidate() override;

private:
  bool changed(const std::vector<Fill>& fills)
  {
    if (fills.size() != m_pens.size())
      return true;
    for (size_t i = 0; i < fills.size(); i++)
    {
      if (fills[i] != m_pens[i]->getFill())
        return true;
    }
    return false;
  }
  std::vector<Ref<FillPenNode>> m_pens;
};

class StackBorderEffectImpl : public GraphicItemEffectNode
{
public:
  StackBorderEffectImpl(VRefCnt* cnt, Ref<GraphicItem> child)
    : GraphicItemEffectNode(cnt, std::move(child))
  {
  }

  Bounds effectBounds() const override
  {
    return m_effectBounds;
  }

  const std::vector<Ref<BorderPenNode>>& borders() const
  {
    return m_pens;
  }

  BorderPenNode* border(int i) const
  {
    if (i >= (int)m_pens.size() || i < 0)
      return nullptr;
    return m_pens[i].get();
  }

  void setBorderStyle(std::vector<Border> borders);

  VGG_CLASS_MAKE(StackBorderEffectImpl);

protected:
  bool changed(const std::vector<Border>& borders)
  {
    if (borders.size() != m_pens.size())
      return true;
    for (size_t i = 0; i < borders.size(); i++)
    {
      if (borders[i] != m_pens[i]->getBorder())
        return true;
    }
    return false;
  }
  Bounds onRevalidate() override;

  Bounds computeFastBounds(const SkRect& bounds) const;

  void onRenderShape(Renderer* renderer, const VShape& shape) override;

private:
  Bounds                          m_effectBounds;
  std::vector<Ref<BorderPenNode>> m_pens;
};

// class DropShadowEffectImpl : public EffectNode
// {
// public:
//   DropShadowEffectImpl(VRefCnt* cnt, Ref<RenderNode> child, const DropShadow& dropShadow)
//     : EffectNode(cnt, std::move(child))
//   {
//   }
//
//   Bounds effectBounds() const override
//   {
//     return m_effectBounds;
//   }
//
//   VGG_CLASS_MAKE(DropShadowEffectImpl);
//
//   Ref<DropShadowEffectImpl> MakeFrom( // NOLINT
//     Ref<RenderNode>                child,
//     const std::vector<DropShadow>& shadows);
//
// protected:
//   Bounds onRevalidate() override;
//
// private:
//   DropShadow m_dropShadow;
//   Bounds     m_effectBounds;
// };
//
// class InnerShadowEffectImpl : public EffectNode
// {
// public:
//   InnerShadowEffectImpl(VRefCnt* cnt, Ref<RenderNode> child, const InnerShadow& innerShadow)
//     : EffectNode(cnt, std::move(child))
//   {
//   }
//   Bounds effectBounds() const override
//   {
//     return m_effectBounds;
//   }
//
//   Ref<InnerShadowEffectImpl> MakeFrom( // NOLINT
//     Ref<RenderNode>                 child,
//     const std::vector<InnerShadow>& shadows);
//
//   VGG_CLASS_MAKE(InnerShadowEffectImpl);
//
// protected:
//   Bounds onRevalidate() override;
//
// private:
//   InnerShadow m_innerShadow;
//   Bounds      m_effectBounds;
// };

} // namespace VGG::layer
