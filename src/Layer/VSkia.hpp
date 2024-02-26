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
#include "Layer/SkSL.hpp"

#include "Math/Algebra.hpp"
#include "Math/Math.hpp"
#include "Utility/Log.hpp"
#include "glm/gtx/transform.hpp"

#include <core/SkClipOp.h>
#include <core/SkColorFilter.h>
#include <core/SkRefCnt.h>
#include <core/SkSamplingOptions.h>
#include <core/SkScalar.h>
#include <include/pathops/SkPathOps.h>
#include <core/SkShader.h>
#include <core/SkPathEffect.h>
#include <include/core/SkBlendMode.h>
#include <include/core/SkImageFilter.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <include/core/SkImage.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkColorMatrix.h>
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

inline std::ostream& operator<<(std::ostream& os, const SkMatrix& mat)
{
  os << "[" << mat.getScaleX() << ", " << mat.getSkewX() << ", " << mat.getTranslateX()
     << std::endl;
  os << mat.getSkewY() << ", " << mat.getScaleY() << ", " << mat.getTranslateY() << std::endl;
  os << mat.getPerspX() << ", " << mat.getPerspY() << ", 1]" << std::endl;
  return os;
}

namespace VGG::layer
{

};
