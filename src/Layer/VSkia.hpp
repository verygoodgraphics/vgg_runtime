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
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Scene.hpp"
#include "Math/Geometry.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"
#include "glm/gtx/transform.hpp"

#include <core/SkRefCnt.h>
#include <core/SkShader.h>
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

#include <vector>
#include <unordered_map>
#include <string>
using namespace VGG;

extern std::unordered_map<std::string, sk_sp<SkImage>> g_skiaImageRepo;

template<class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

#define SWITCH_MAP_ITEM_BEGIN(var)                                                                 \
  switch (var)                                                                                     \
  {

#define SWITCH_MAP_ITEM_DEF(from, to)                                                              \
  case from:                                                                                       \
    return to;

#define SWITCH_MAP_ITEM_DEF_NULL(from) case from:;

#define SWITCH_MAP_ITEM_END(fallback)                                                              \
  default:                                                                                         \
    return fallback;                                                                               \
    }

inline SkRect toSkRect(const VGG::Bound& bound)
{
  const auto& b = bound;
  return SkRect{ b.topLeft().x, b.topLeft().y, b.bottomRight().x, b.bottomRight().y };
}

inline SkMatrix toSkMatrix(const glm::mat3& mat)
{
  SkMatrix skMatrix;
  skMatrix.setAll(mat[0][0],
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
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Justify,
                      skia::textlayout::TextAlign::kJustify);
  SWITCH_MAP_ITEM_DEF(VGG::ETextHorizontalAlignment::HA_Center,
                      skia::textlayout::TextAlign::kCenter);
  SWITCH_MAP_ITEM_DEF_NULL(VGG::ETextHorizontalAlignment::HA_Natural)
  SWITCH_MAP_ITEM_END(skia::textlayout::TextAlign::kLeft)
}

inline SkFontStyle toSkFontStyle(const std::string_view& subFamilyName)
{
  if (subFamilyName == "Bold")
  {
    return SkFontStyle::Bold();
  }
  else if (subFamilyName == "Regular")
  {
    return SkFontStyle::Normal();
  }
  else if (subFamilyName == "ExtraBold" || subFamilyName == "Extra Bold" ||
           subFamilyName == "Extra-Bold")
  {
    return SkFontStyle(SkFontStyle::kExtraBold_Weight,
                       SkFontStyle::kNormal_Width,
                       SkFontStyle::kUpright_Slant);
  }
  else if (subFamilyName == "Bold Italic" || subFamilyName == "BoldItalic" ||
           subFamilyName == "Bold-Italic")
  {
    return SkFontStyle::BoldItalic();
  }
  else
  {
    return SkFontStyle::Normal();
  }
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
  std::visit(Overloaded{ [&](const PatternFill& p) { shader = makeFillPattern(bound, p); },
                         [&](const PatternFit& p) { shader = makeFitPattern(bound, p); },
                         [&](const PatternStretch& p) { shader = makeStretchPattern(bound, p); },
                         [&](const PatternTile& p) { shader = makeTilePattern(bound, p); } },
             pattern.instance);
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

inline std::ostream& operator<<(std::ostream& os, const SkMatrix& mat)
{
  os << "[" << mat.getScaleX() << ", " << mat.getSkewX() << ", " << mat.getTranslateX()
     << std::endl;
  os << mat.getSkewY() << ", " << mat.getScaleY() << ", " << mat.getTranslateY() << std::endl;
  os << mat.getPerspX() << ", " << mat.getPerspY() << ", 1]" << std::endl;
  return os;
}

inline float calcRotationAngle(const glm::mat3& mat)
{
  glm::vec3  v{ 1, 0, 0 };
  auto       r = mat * v;
  const auto v1 = glm::vec2{ r.x, r.y };
  auto       d = glm::dot(glm::vec2{ 1, 0 }, v1);
  float      cosAngle = d / glm::length(v1);
  float      radian = acos(cosAngle);
  return radian * 180.0 / 3.141592653;
}

inline float calcRotationAngle(const SkMatrix& mat)
{
  SkVector r;
  mat.mapVector(1, 0, &r);
  float d = r.dot({ 1, 0 });
  float cosAngle = r.dot({ 1, 0 }) / r.length();
  float radian = acos(cosAngle);
  return radian * 180.0 / 3.141592653;
}

inline SkColor nodeType2Color(ObjectType type)
{
  switch (type)
  {
    case ObjectType::VGG_PATH:
      return SK_ColorRED;
    case ObjectType::VGG_IMAGE:
      return SK_ColorRED;
    case ObjectType::VGG_GROUP:
      return SK_ColorRED;
    case ObjectType::VGG_TEXT:
      return SK_ColorRED;
    case ObjectType::VGG_ARTBOARD:
      return SK_ColorRED;
    case ObjectType::VGG_LAYER:
      return SK_ColorRED;
    case ObjectType::VGG_MASTER:
      return SK_ColorRED;
    case ObjectType::VGG_CONTOUR:
      return SK_ColorYELLOW;
    default:
      return SK_ColorRED;
  }
}

inline uint8_t zorder2Alpha(int zorder)
{
  if (zorder < 0)
  {
    return 255;
  }
  return float(zorder) / 5 * 255.0;
}
