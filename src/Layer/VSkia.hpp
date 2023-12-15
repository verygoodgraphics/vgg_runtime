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
#include "Layer/Core/VUtils.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Scene.hpp"
#include "Math/Algebra.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"
#include "glm/gtx/transform.hpp"

#include <core/SkRefCnt.h>
#include <core/SkShader.h>
#include <core/SkPathEffect.h>
#include <include/core/SkBlendMode.h>
#include <include/core/SkImageFilter.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/core/SkImage.h>
#include <include/effects/SkImageFilters.h>
#include <include/pathops/SkPathOps.h>
#include <include/core/SkFontStyle.h>
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/TextStyle.h>

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
  SWITCH_MAP_ITEM_DEF(LJ_Miter, SkPaint::kMiter_Join)
  SWITCH_MAP_ITEM_DEF(LJ_Round, SkPaint::kRound_Join)
  SWITCH_MAP_ITEM_DEF(LJ_Bevel, SkPaint::kBevel_Join)
  SWITCH_MAP_ITEM_END(SkPaint::kMiter_Join)
}

inline SkBlendMode toSkBlendMode(EBlendMode mode)
{
  SWITCH_MAP_ITEM_BEGIN(mode)
  SWITCH_MAP_ITEM_DEF(BM_Normal, SkBlendMode::kSrcOver)
  SWITCH_MAP_ITEM_DEF(BM_Darken, SkBlendMode::kDarken)
  SWITCH_MAP_ITEM_DEF(BM_Multiply, SkBlendMode::kMultiply)
  SWITCH_MAP_ITEM_DEF(BM_Color_burn, SkBlendMode::kColorBurn)
  SWITCH_MAP_ITEM_DEF(BM_Lighten, SkBlendMode::kLighten)
  SWITCH_MAP_ITEM_DEF(BM_Screen, SkBlendMode::kScreen)
  SWITCH_MAP_ITEM_DEF(BM_Color_dodge, SkBlendMode::kColorDodge)
  SWITCH_MAP_ITEM_DEF(BM_Overlay, SkBlendMode::kOverlay)
  SWITCH_MAP_ITEM_DEF(BM_Soft_light, SkBlendMode::kSoftLight)
  SWITCH_MAP_ITEM_DEF(BM_Hard_light, SkBlendMode::kHardLight)
  SWITCH_MAP_ITEM_DEF(BM_Difference, SkBlendMode::kDifference)
  SWITCH_MAP_ITEM_DEF(BM_Exclusion, SkBlendMode::kExclusion)
  SWITCH_MAP_ITEM_DEF(BM_Hue, SkBlendMode::kHue)
  SWITCH_MAP_ITEM_DEF(BM_Saturation, SkBlendMode::kSaturation)
  SWITCH_MAP_ITEM_DEF(BM_Color, SkBlendMode::kColor)
  SWITCH_MAP_ITEM_DEF(BM_Luminosity, SkBlendMode::kColor)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Plus_darker)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Plus_lighter)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Blend_divide)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Blend_subtraction)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Darker_color)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Dissolve)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Hard_mix)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighter_color)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighten_burn)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighten_dodge)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Lighten_light)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Pass_through)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Pin_Light)
  SWITCH_MAP_ITEM_DEF_NULL(BM_Vivid_light)
  SWITCH_MAP_ITEM_END(SkBlendMode::kSrcOver)
  return SkBlendMode::kSrcOver;
}

inline SkPaint::Cap toSkPaintCap(VGG::ELineCap cap)
{
  SWITCH_MAP_ITEM_BEGIN(cap)
  SWITCH_MAP_ITEM_DEF(LC_Butt, SkPaint::kButt_Cap)
  SWITCH_MAP_ITEM_DEF(LC_Round, SkPaint::kRound_Cap)
  SWITCH_MAP_ITEM_DEF(LC_Square, SkPaint::kSquare_Cap)
  SWITCH_MAP_ITEM_END(SkPaint::kButt_Cap)
}

inline skia::textlayout::TextAlign toSkTextAlign(ETextVerticalAlignment align)
{
  SWITCH_MAP_ITEM_BEGIN(align)
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_Top, skia::textlayout::TextAlign::kStart);
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_Bottom, skia::textlayout::TextAlign::kEnd);
  SWITCH_MAP_ITEM_DEF(VGG::ETextVerticalAlignment::VA_Center, skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kStart)
}

inline skia::textlayout::TextAlign toSkTextAlign(ETextHorizontalAlignment align)
{
  SWITCH_MAP_ITEM_BEGIN(align)
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Left, skia::textlayout::TextAlign::kLeft);
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Right, skia::textlayout::TextAlign::kRight);
  SWITCH_MAP_ITEM_DEF(
    VGG::ETextHorizontalAlignment::HA_Justify,
    skia::textlayout::TextAlign::kJustify);
  SWITCH_MAP_ITEM_DEF(
    VGG::ETextHorizontalAlignment::HA_Center,
    skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_DEF_NULL(VGG::ETextHorizontalAlignment::HA_Natural)
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kLeft)
}

inline SkFontStyle toSkFontStyle(const TextStyleAttr& attr)
{
  SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
  if (
    attr.subFamilyName.find("Italic") != std::string::npos ||
    attr.fontName.find("Italic") != std::string::npos)
    slant = SkFontStyle::kItalic_Slant;
  return { SkScalarRoundToInt(attr.fontWeight),
           SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(attr.fontWidth),
           slant };
}

inline SkPathOp toSkPathOp(VGG::EBoolOp blop)
{
  SWITCH_MAP_ITEM_BEGIN(blop)
  SWITCH_MAP_ITEM_DEF(VGG::BO_Union, SkPathOp::kUnion_SkPathOp)
  SWITCH_MAP_ITEM_DEF(BO_Substraction, SkPathOp::kDifference_SkPathOp)
  SWITCH_MAP_ITEM_DEF(VGG::BO_Intersection, SkPathOp::kIntersect_SkPathOp)
  SWITCH_MAP_ITEM_DEF(VGG::BO_Exclusion, SkPathOp::kXOR_SkPathOp)
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
  SkSamplingOptions opt;
  SkTileMode        modeX = SkTileMode::kDecal;
  SkTileMode        modeY = SkTileMode::kDecal;
  const auto        mat = toSkMatrix(m);
  return img->makeShader(modeX, modeY, opt, &mat);
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
  SkSamplingOptions opt;
  SkTileMode        modeX = SkTileMode::kDecal;
  SkTileMode        modeY = SkTileMode::kDecal;
  const auto        mat = toSkMatrix(m);
  return img->makeShader(modeX, modeY, opt, &mat);
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
  SkSamplingOptions opt;
  SkTileMode        modeX = SkTileMode::kDecal;
  SkTileMode        modeY = SkTileMode::kDecal;
  return img->makeShader(modeX, modeY, opt, &mat);
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
  SkSamplingOptions opt;
  return img->makeShader(modeX, modeY, opt, &mat);
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

  auto f = bound.map(bound.size() * g.from);
  auto t = bound.map(bound.size() * g.to);
  auto start = glm::mix(f, t, minPosition);
  auto end = glm::mix(f, t, maxPosition);

  SkPoint  center{ (SkScalar)start.x, (SkScalar)start.y };
  SkScalar r = glm::distance(end, start);

  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;
  for (auto it = g.stops.begin(); it != g.stops.end(); ++it)
  {
    colors.push_back(it->color);
    auto p = it->position;
    positions.push_back((p - minPosition) / (maxPosition - minPosition));
  }

  auto theta = [](const glm::vec2& from, const glm::vec2& to)
  {
    const auto d = to - from;
    return vectorSign({ 1.0f, 0.f }, d) * std::atan2(d.y, d.x);
  };
  SkMatrix mat;
  if (auto p = std::get_if<float>(&g.ellipse); p)
  {
    mat.postTranslate(-start.x, -start.y);
    mat.postScale(*p, 1.0);
    mat.postRotate(rad2deg(theta(f, t)));
    mat.postTranslate(start.x, start.y);
  }
  else if (auto p = std::get_if<glm::vec2>(&g.ellipse); p)
  {
    auto pp = bound.map(bound.size() * (*p));
    mat.postTranslate(-start.x, -start.y);
    const auto a = glm::distance(f, t);
    const auto b = glm::distance(f, pp);
    const auto ratio = (a == 0.f) ? 0.f : b / a;
    mat.postScale(ratio, 1.0);
    mat.postRotate(rad2deg(theta(f, t)));
    mat.postTranslate(start.x, start.y);
  }
  return SkGradientShader::MakeRadial(
    center,
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

  const auto minPosition = g.stops.front().position;
  const auto maxPosition = g.stops.back().position;

  const glm::vec2 from = { g.from[0], g.from[1] };
  const glm::vec2 to = { g.to[0], g.to[1] };

  auto f = bound.map(bound.size() * from);
  auto t = bound.map(bound.size() * to);

  auto center = f;

  std::vector<SkColor>  colors;
  std::vector<SkScalar> positions;

  auto minPosColor = g.stops.front().color;
  auto maxPosColor = g.stops.back().color;

  size_t sz = g.stops.size();
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

  SkMatrix mat = SkMatrix::I();

  auto theta = [](const glm::vec2& from, const glm::vec2& to)
  {
    const auto d = to - from;
    return vectorSign({ 1.0f, 0.f }, d) * std::atan2(d.y, d.x);
  };
  // mat.setRotate(glm::degrees(theta(f, t)), center.x, center.y);
  if (auto p = std::get_if<float>(&g.ellipse); p)
  {
    mat.postTranslate(-center.x, -center.y);
    mat.postScale(*p, 1.0);
    mat.postRotate(-rad2deg(theta(f, t)));
    mat.postTranslate(center.x, center.y);
  }
  else if (auto p = std::get_if<glm::vec2>(&g.ellipse); p)
  {
    auto pp = bound.map(bound.size() * (*p));
    mat.postTranslate(-center.x, -center.y);
    // const auto a = glm::distance(f, t);
    // const auto b = glm::distance(f, pp);
    // const auto ratio = (a == 0.f) ? 0.f : b / a;
    mat.postScale(1.0, -vectorSign(pp - f, t - f));
    mat.postRotate(-rad2deg(theta(f, t)));
    mat.postTranslate(center.x, center.y);
  }
  return SkGradientShader::MakeSweep(
    center.x,
    center.y,
    colors.data(),
    positions.data(),
    sz,
    0,
    &mat);
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
    case VGG::BM_Blend_divide:
      SkImageFilters::Arithmetic(0, 0, 0, 0, true, nullptr, nullptr);
    default:
      return nullptr;
  }
}

inline void populateSkPaint(
  const FillType&       fillType,
  const ContextSetting& st,
  const Bound&          bound,
  SkPaint&              paint)
{
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
}

inline void populateSkPaint(const Border& border, const Bound& bound, SkPaint& paint)
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
