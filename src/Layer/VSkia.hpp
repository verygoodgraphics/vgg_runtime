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
#include "Layer/AttrSerde.hpp"
#include "Layer/Effects.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Scene.hpp"
#include "Layer/SkSL.hpp"
#include "Math/Algebra.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"
#include "glm/gtx/transform.hpp"

#include <core/SkColorFilter.h>
#include <core/SkRefCnt.h>
#include <core/SkScalar.h>
#include <core/SkShader.h>
#include <core/SkPathEffect.h>
#include <include/core/SkBlendMode.h>
#include <include/core/SkImageFilter.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/core/SkImage.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkColorMatrix.h>
#include <include/pathops/SkPathOps.h>
#include <include/core/SkFontStyle.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/TextStyle.h>

#include <optional>
#include <src/core/SkFontDescriptor.h>
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
using namespace VGG;

extern std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo;

inline SkRect toSkRect(const VGG::Bound& bound)
{
  const auto& b = bound;
  return SkRect{ b.topLeft().x, b.topLeft().y, b.bottomRight().x, b.bottomRight().y };
}

inline SkMatrix toSkMatrix(const glm::mat3& mat)
{
  SkMatrix skMatrix;
  skMatrix.setAll(
    mat[0][0],
    mat[1][0],
    mat[2][0],
    mat[0][1],
    mat[1][1],
    mat[2][1],
    mat[0][2],
    mat[1][2],
    mat[2][2]);
  return skMatrix;
}
inline SkPaint::Join toSkPaintJoin(VGG::ELineJoin join)
{
  SWITCH_MAP_ITEM_BEGIN(join)
  SWITCH_MAP_ITEM_DEF(LJ_MITER, SkPaint::kMiter_Join)
  SWITCH_MAP_ITEM_DEF(LJ_ROUND, SkPaint::kRound_Join)
  SWITCH_MAP_ITEM_DEF(LJ_BEVEL, SkPaint::kBevel_Join)
  SWITCH_MAP_ITEM_END(SkPaint::kMiter_Join)
}

inline std::optional<std::variant<SkBlendMode, sk_sp<SkBlender>>> toSkBlendMode(EBlendMode mode)
{
  SWITCH_MAP_ITEM_BEGIN(mode)
  SWITCH_MAP_ITEM_DEF(BM_NORMAL, SkBlendMode::kSrcOver)
  SWITCH_MAP_ITEM_DEF(BM_DARKEN, SkBlendMode::kDarken)
  SWITCH_MAP_ITEM_DEF(BM_MULTIPLY, SkBlendMode::kMultiply)
  SWITCH_MAP_ITEM_DEF(BM_COLOR_BURN, SkBlendMode::kColorBurn)
  SWITCH_MAP_ITEM_DEF(BM_LIGHTEN, SkBlendMode::kLighten)
  SWITCH_MAP_ITEM_DEF(BM_SCREEN, SkBlendMode::kScreen)
  SWITCH_MAP_ITEM_DEF(BM_COLOR_DODGE, SkBlendMode::kColorDodge)
  SWITCH_MAP_ITEM_DEF(BM_OVERLAY, SkBlendMode::kOverlay)
  SWITCH_MAP_ITEM_DEF(BM_SOFT_LIGHT, SkBlendMode::kSoftLight)
  SWITCH_MAP_ITEM_DEF(BM_HARD_LIGHT, SkBlendMode::kHardLight)
  SWITCH_MAP_ITEM_DEF(BM_DIFFERENCE, SkBlendMode::kDifference)
  SWITCH_MAP_ITEM_DEF(BM_EXCLUSION, SkBlendMode::kExclusion)
  SWITCH_MAP_ITEM_DEF(BM_HUE, SkBlendMode::kHue)
  SWITCH_MAP_ITEM_DEF(BM_SATURATION, SkBlendMode::kSaturation)
  SWITCH_MAP_ITEM_DEF(BM_COLOR, SkBlendMode::kColor)
  SWITCH_MAP_ITEM_DEF(BM_LUMINOSITY, SkBlendMode::kLuminosity)
  SWITCH_MAP_ITEM_DEF(BM_PASS_THROUGHT, std::nullopt)
  SWITCH_MAP_ITEM_DEF_NULL(BM_PLUS_DARKER)
  SWITCH_MAP_ITEM_DEF_NULL(BM_PlUS_LIGHTER)
  SWITCH_MAP_ITEM_DEF_NULL(BM_BLEND_DIVIDE)
  SWITCH_MAP_ITEM_DEF_NULL(BM_BLEND_SUBSTRACTION)
  SWITCH_MAP_ITEM_DEF_NULL(BM_DARKER_COLOR)
  SWITCH_MAP_ITEM_DEF_NULL(BM_DISSOLVE)
  SWITCH_MAP_ITEM_DEF_NULL(BM_HARD_MIX)
  SWITCH_MAP_ITEM_DEF_NULL(BM_LIGHTER_COLOR)
  SWITCH_MAP_ITEM_DEF_NULL(BM_LIGHTEN_BURN)
  SWITCH_MAP_ITEM_DEF_NULL(BM_LIGHTEN_DODGE)
  SWITCH_MAP_ITEM_DEF_NULL(BM_LIGHTEN_LIGHT)
  SWITCH_MAP_ITEM_DEF_NULL(BM_PIN_LIGHT)
  SWITCH_MAP_ITEM_DEF_NULL(BM_VIVID_LIGHT)
  SWITCH_MAP_ITEM_END(SkBlendMode::kSrcOver)
  return SkBlendMode::kSrcOver;
}

inline SkPaint::Cap toSkPaintCap(VGG::ELineCap cap)
{
  SWITCH_MAP_ITEM_BEGIN(cap)
  SWITCH_MAP_ITEM_DEF(LC_BUTT, SkPaint::kButt_Cap)
  SWITCH_MAP_ITEM_DEF(LC_ROUND, SkPaint::kRound_Cap)
  SWITCH_MAP_ITEM_DEF(LC_SQUARE, SkPaint::kSquare_Cap)
  SWITCH_MAP_ITEM_END(SkPaint::kButt_Cap)
}

inline skia::textlayout::TextAlign toSkTextAlign(ETextVerticalAlignment align)
{
  SWITCH_MAP_ITEM_BEGIN(align)
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_TOP, skia::textlayout::TextAlign::kStart);
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_BOTTOM, skia::textlayout::TextAlign::kEnd);
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_CENTER, skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kStart)
}

inline skia::textlayout::TextAlign toSkTextAlign(ETextHorizontalAlignment align)
{
  SWITCH_MAP_ITEM_BEGIN(align)
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_LEFT, skia::textlayout::TextAlign::kLeft);
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_RIGHT, skia::textlayout::TextAlign::kRight);
  SWITCH_MAP_ITEM_DEF(
    VGG::ETextHorizontalAlignment::HA_JUSTIFY,
    skia::textlayout::TextAlign::kJustify);
  SWITCH_MAP_ITEM_DEF(
    VGG::ETextHorizontalAlignment::HA_CENTER,
    skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_DEF_NULL(VGG::ETextHorizontalAlignment::HA_NATURAL)
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kLeft)
}

inline SkPathOp toSkPathOp(VGG::EBoolOp blop)
{
  SWITCH_MAP_ITEM_BEGIN(blop)
  SWITCH_MAP_ITEM_DEF(VGG::BO_UNION, SkPathOp::kUnion_SkPathOp)
  SWITCH_MAP_ITEM_DEF(BO_SUBSTRACTION, SkPathOp::kDifference_SkPathOp)
  SWITCH_MAP_ITEM_DEF(VGG::BO_INTERSECTION, SkPathOp::kIntersect_SkPathOp)
  SWITCH_MAP_ITEM_DEF(VGG::BO_EXECLUSION, SkPathOp::kXOR_SkPathOp)
  SWITCH_MAP_ITEM_END(SkPathOp::kUnion_SkPathOp)
}

inline sk_sp<SkImage> loadImage(const std::string& imageGUID, const ResourceRepo& repo)
{
  sk_sp<SkImage> image;
  if (imageGUID.empty())
    return image;
  std::string guid = imageGUID;
  if (auto pos = guid.find("./"); pos != std::string::npos && pos == 0)
  {
    // remove current dir notation
    guid = guid.substr(2);
  }
  if (auto it = g_skiaImageRepo.find(guid); it != g_skiaImageRepo.end())
  {
    image = it->second;
  }
  else
  {
    auto repo = Scene::getResRepo();
    if (auto it = repo.find(guid); it != repo.end())
    {
      auto data = SkData::MakeWithCopy(it->second.data(), it->second.size());
      if (!data)
      {
        WARN("Make SkData failed");
        return image;
      }
      sk_sp<SkImage> skImage = SkImages::DeferredFromEncodedData(data);
      if (!skImage)
      {
        WARN("Make SkImage failed.");
        return image;
      }
      g_skiaImageRepo[guid] = skImage;
      image = skImage;
    }
    else
    {
      WARN("Cannot find %s from resources repository", guid.c_str());
    }
  }
  return image;
}

sk_sp<SkColorFilter> makeColorFilter(const ImageFilter& imageFilter);

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

inline sk_sp<SkShader> makeFitPattern(const Bound& bound, const PatternFit& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());
  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bound.width();
  float       height = bound.height();
  float       sx = (float)width / mi.width();
  float       sy = (float)height / mi.height();
  auto        m = glm::mat3{ 1.0 };
  float       s = std::min(sx, sy);
  if (sx < sy)
  {
    m = glm::translate(m, { 0, (height - s * mi.height()) / 2 });
  }
  else
  {
    m = glm::translate(m, { (width - s * mi.width()) / 2, 0 });
  }
  m = glm::scale(m, { s, s });
  m = glm::translate(m, { mi.width() / 2, mi.height() / 2 });
  m = glm::rotate(m, p.rotation);
  m = glm::translate(m, { -mi.width() / 2, -mi.height() / 2 });
  SkSamplingOptions opt(SkFilterMode::kLinear, SkMipmapMode::kNearest);
  SkTileMode        modeX = SkTileMode::kDecal;
  SkTileMode        modeY = SkTileMode::kDecal;
  const auto        mat = toSkMatrix(m);
  auto              shader = img->makeShader(modeX, modeY, opt, &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}
inline sk_sp<SkShader> makeFillPattern(const Bound& bound, const PatternFill& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());

  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bound.width();
  float       height = bound.height();
  float       sx = (float)width / mi.width();
  float       sy = (float)height / mi.height();
  auto        m = glm::mat3{ 1.0 };
  const float s = std::max(sx, sy);
  if (sx > sy)
  {
    m = glm::translate(m, { 0, (height - s * mi.height()) / 2.f });
  }
  else
  {
    m = glm::translate(m, { (width - s * mi.width()) / 2.f, 0 });
  }
  m = glm::scale(m, { s, s });
  m = glm::translate(m, { mi.width() / 2, mi.height() / 2 });
  m = glm::rotate(m, p.rotation);
  m = glm::translate(m, { -mi.width() / 2, -mi.height() / 2 });
  SkSamplingOptions opt(SkFilterMode::kLinear, SkMipmapMode::kNearest);
  SkTileMode        modeX = SkTileMode::kDecal;
  SkTileMode        modeY = SkTileMode::kDecal;
  const auto        mat = toSkMatrix(m);
  auto              shader = img->makeShader(modeX, modeY, opt, &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

inline sk_sp<SkShader> makeStretchPattern(const Bound& bound, const PatternStretch& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());
  if (!img)
    return nullptr;
  SkImageInfo mi = img->imageInfo();
  float       width = bound.width();
  float       height = bound.height();
  auto        m = glm::mat3{ 1.0 };
  m = glm::scale(m, { width, height });
  m *= p.transform.matrix();
  m = glm::scale(m, { 1.f / mi.width(), 1.f / mi.height() });
  const auto        mat = toSkMatrix(m);
  SkSamplingOptions opt(SkFilterMode::kLinear, SkMipmapMode::kNearest);
  SkTileMode        modeX = SkTileMode::kDecal;
  SkTileMode        modeY = SkTileMode::kDecal;
  auto              shader = img->makeShader(modeX, modeY, opt, &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
}

inline sk_sp<SkShader> makeTilePattern(const Bound& bound, const PatternTile& p)
{
  auto img = loadImage(p.guid, Scene::getResRepo());

  if (!img)
    return nullptr;
  SkTileMode modeX = SkTileMode::kDecal;
  SkTileMode modeY = SkTileMode::kDecal;
  if (p.mode == VGG::TILE_VERTICAL)
  {
    modeY = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  if (p.mode == VGG::TILE_HORIZONTAL)
  {
    modeX = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  if (p.mode == VGG::TILE_BOTH)
  {
    modeY = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
    modeX = p.mirror ? SkTileMode::kMirror : SkTileMode::kRepeat;
  }
  auto m = glm::mat3{ 1.0 };
  m = glm::rotate(m, p.rotation);
  m = glm::scale(m, { p.scale, p.scale });
  const auto        mat = toSkMatrix(m);
  SkSamplingOptions opt(SkFilterMode::kLinear, SkMipmapMode::kNearest);
  auto              shader = img->makeShader(modeX, modeY, opt, &mat);
  if (auto colorFilter = makeColorFilter(p.imageFilter); shader && colorFilter)
  {
    return shader->makeWithColorFilter(colorFilter);
  }
  return shader;
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

inline sk_sp<SkShader> makeGradientLinear(const Bound& bound, const GradientLinear& g)
{
  if (g.stops.empty())
    return nullptr;
  auto minPosition = g.stops.front().position;
  auto maxPosition = g.stops.back().position;
  // clampPairByLimits(minPosition, maxPosition, 0.f, 1.f, 0.0001f);

  auto f = bound.map(bound.size() * g.from);
  auto t = bound.map(bound.size() * g.to);
  auto start = glm::mix(f, t, minPosition);
  auto end = glm::mix(f, t, maxPosition);

  SkPoint pts[2] = {
    { (SkScalar)start.x, (SkScalar)start.y },
    { (SkScalar)end.x, (SkScalar)end.y },
  };
  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;
  for (auto it = g.stops.begin(); it != g.stops.end(); ++it)
  {
    colors.push_back(it->color);
    auto p = it->position;
    positions.push_back((p - minPosition) / (maxPosition - minPosition));
  }
  SkMatrix mat = SkMatrix::I();
  auto     s = SkGradientShader::MakeLinear(
    pts,
    colors.data(),
    positions.data(),
    colors.size(),
    SkTileMode::kClamp,
    0,
    &mat);
  return s;
}

template<typename G>
inline sk_sp<SkShader> makeGradientRadial(const Bound& bound, const G& g)
{
  if (g.stops.empty())
    return nullptr;
  auto minPosition = g.stops.front().position;
  auto maxPosition = g.stops.back().position;

  auto     f = bound.map(bound.size() * g.from);
  auto     t = bound.map(bound.size() * g.to);
  auto     start = glm::mix(f, t, minPosition);
  auto     end = glm::mix(f, t, maxPosition);
  SkScalar r = glm::distance(end, start);

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
    { start.x, start.y },
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

inline sk_sp<SkImageFilter> makeBlendModeFilter(EBlendMode blendMode)
{
  switch (blendMode)
  {
    case VGG::BM_BLEND_DIVIDE:
      SkImageFilters::Arithmetic(0, 0, 0, 0, true, nullptr, nullptr);
    default:
      return nullptr;
  }
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
  paint.setPathEffect(
    SkDashPathEffect::Make(border.dashedPattern.data(), border.dashedPattern.size(), 0));
  paint.setStrokeJoin(toSkPaintJoin(border.lineJoinStyle));
  paint.setStrokeCap(toSkPaintCap(border.lineCapStyle));
  paint.setStrokeMiter(border.miterLimit);
  paint.setStrokeWidth(border.thickness);
  populateSkPaint(border.type, border.contextSettings, bound, paint);
}

inline std::ostream& operator<<(std::ostream& os, const SkMatrix& mat)
{
  os << "[" << mat.getScaleX() << ", " << mat.getSkewX() << ", " << mat.getTranslateX()
     << std::endl;
  os << mat.getSkewY() << ", " << mat.getScaleY() << ", " << mat.getTranslateY() << std::endl;
  os << mat.getPerspX() << ", " << mat.getPerspY() << ", 1]" << std::endl;
  return os;
}
