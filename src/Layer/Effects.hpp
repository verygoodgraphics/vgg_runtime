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

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VUtils.hpp"

#include "Layer/LRUCache.hpp"
#include "Layer/LayerCache.h"
#include "Layer/SkSL.hpp"
#include "Layer/VSkia.hpp"

#include <core/SkBlender.h>
#include <include/effects/SkRuntimeEffect.h>
#include <include/core/SkSamplingOptions.h>

namespace VGG::layer
{

sk_sp<SkColorFilter>   makeColorFilter(const ImageFilter& imageFilter);
sk_sp<SkShader>        makeFitPattern(const Bound& bound, const PatternFit& p);
sk_sp<SkShader>        makeFillPattern(const Bound& bound, const PatternFill& p);
sk_sp<SkShader>        makeStretchPattern(const Bound& bound, const PatternStretch& p);
sk_sp<SkShader>        makeTilePattern(const Bound& bound, const PatternTile& p);
sk_sp<SkShader>        makeGradientLinear(const Bound& bound, const GradientLinear& g);
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

inline sk_sp<SkShader> makePatternShader(const Bound& bound, const Pattern& pattern)
{
  sk_sp<SkShader> shader;
  std::visit(
    Overloaded{ [&](const PatternFill& p) { shader = makeFillPattern(bound, p); },
                [&](const PatternFit& p) { shader = makeFitPattern(bound, p); },
                [&](const PatternStretch& p) { shader = makeStretchPattern(bound, p); },
                [&](const PatternTile& p) { shader = makeTilePattern(bound, p); } },
    pattern.instance);
  return shader;
}

inline SkMatrix makeMatrix(
  const Bound&                          bound,
  const glm::vec2&                      f,
  const glm::vec2&                      t,
  const std::variant<float, glm::vec2>& ellipse)
{
  const auto from = bound.map(bound.size() * f);
  const auto to = bound.map(bound.size() * t);

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
                  auto       pp = bound.map(bound.size() * p);
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
inline sk_sp<SkShader> makeGradientRadial(const Bound& bound, const G& g)
{
  if (g.stops.empty())
    return nullptr;

  auto       f = bound.map(bound.size() * g.from);
  auto       t = bound.map(bound.size() * g.to);
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
  auto mat = makeMatrix(bound, g.from, g.to, g.ellipse);
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
inline sk_sp<SkShader> makeGradientAngular(const Bound& bound, const G& g)
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

  const auto mat = makeMatrix(bound, g.from, g.to, g.ellipse);
  const auto from = bound.map(bound.size() * g.from);
  return SkGradientShader::MakeSweep(from.x, from.y, colors.data(), positions.data(), sz, 0, &mat);
}

inline sk_sp<SkShader> makeGradientShader(const Bound& bound, const Gradient& gradient)
{
  sk_sp<SkShader> shader;
  std::visit(
    Overloaded{
      [&](const GradientLinear& p) { shader = makeGradientLinear(bound, p); },
      [&](const GradientRadial& p) { shader = makeGradientRadial(bound, p); },
      [&](const GradientAngular& p) { shader = makeGradientAngular(bound, p); },
      [&](const GradientDiamond& p) { shader = makeGradientRadial(bound, p); },
      [&](const GradientBasic& p) { // TODO::
      },
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
  Bound bound{ rect.x(), rect.y(), rect.width(), rect.height() };
  std::visit(
    Overloaded{ [&](const Gradient& g)
                {
                  paint.setShader(makeGradientShader(bound, g));
                  paint.setAlphaf(st.opacity);
                },
                [&](const Color& c)
                {
                  paint.setColor(c);
                  paint.setAlphaf(c.a * st.opacity);
                },
                [&](const Pattern& p)
                {
                  paint.setShader(makePatternShader(bound, p));
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

inline void populateSkPaint(const Border& border, const SkRect& bound, SkPaint& paint)
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
  populateSkPaint(border.type, border.contextSettings, bound, paint);
}

void                     setGlobalSamplingOptions(const SkSamplingOptions& opt);
const SkSamplingOptions& getGlobalSamplingOptions();

} // namespace VGG::layer
