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

#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VColor.hpp"
#include "Math/Hash.hpp"
#include "Math/Math.hpp"
#include "Layer/Core/VBound.hpp"
#include "Layer/Core/VType.hpp"
#include "Utility/Log.hpp"

#include <include/effects/SkGradientShader.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/core/SkMatrix.h>
#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <algorithm>
#include <vector>
#include <stdint.h>
#include <variant>

namespace VGG
{

struct ContextSetting
{
  EBlendMode    blendMode{ BM_Normal };
  float         opacity{ 1.0 };
  bool          isolateBlending{ false };
  EKnockoutType transparencyKnockoutGroup{ KT_Off };
};

struct PatternTile
{
  std::string      guid;
  ETilePatternType mode{ TILE_BOTH };
  bool             mirror{ true };
  float            rotation{ 0.f };
  float            scale{ 1.f };
};

struct PatternStretch
{
  std::string      guid;
  layer::Transform transform;
  bool             clip{ false };
};

struct PatternFill
{
  std::string guid;
  float       rotation;
};

struct PatternFit
{
  std::string guid;
  float       rotation;
};

struct Pattern
{
  std::variant<PatternFill, PatternFit, PatternStretch, PatternTile> instance;
};

struct AlphaMask
{
  std::string   id;
  AlphaMaskType type{ AM_ALPHA };
  bool          crop{ true };
};

struct GradientStop
{
  Color color{ 1., 1., 1., 1. };
  float position{ 1.0 }; // [0,1]
  float midPoint;
};

struct GradientLinear
{
  glm::vec2                 from;
  glm::vec2                 to;
  std::vector<GradientStop> stops;
  bool                      invert{ false };
};

struct GradientRadial
{
  glm::vec2                      from;
  glm::vec2                      to;
  std::vector<GradientStop>      stops;
  std::variant<float, glm::vec2> ellipse;
  bool                           invert{ false };
};

struct GradientDiamond
{
  glm::vec2                      from;
  glm::vec2                      to;
  std::vector<GradientStop>      stops;
  std::variant<float, glm::vec2> ellipse;
  bool                           invert{ false };
};

struct GradientAngular
{
  glm::vec2                      from;
  glm::vec2                      to;
  std::vector<GradientStop>      stops;
  std::variant<float, glm::vec2> ellipse;
  bool                           invert{ false };
};

struct GradientBasic
{
  // Ai specific
};

struct Gradient
{
  std::variant<GradientLinear, GradientRadial, GradientAngular, GradientBasic, GradientDiamond>
    instance;
};

struct Border
{
  ContextSetting          contextSettings;
  double                  dashedOffset;
  std::vector<float>      dashedPattern;
  EPathFillType           fillType; // TODO:
  double                  flat;
  bool                    isEnabled;
  ELineCap                lineCapStyle;
  ELineJoin               lineJoinStyle;
  double                  miterLimit;
  EPathPosition           position;
  int64_t                 style;
  double                  thickness;
  std::optional<Color>    color;
  std::optional<Gradient> gradient;
  std::optional<Pattern>  pattern;
};

struct Shadow
{
  float          blur;
  Color          color;
  ContextSetting contextSettings;
  bool           inner;
  bool           isEnabled;
  float          offsetX;
  float          offsetY;
  float          spread;
};

struct Blur
{
  EBlurType blurType;
  float     radius;
  float     motionAngle;
  glm::vec2 center;
  float     saturation;
  bool      isEnabled;
};

struct Fill
{
  bool                    isEnabled{ true };
  Color                   color;
  EPathFillType           fillType{};
  ContextSetting          contextSettings{};
  std::optional<Gradient> gradient{ std::nullopt };
  std::optional<Pattern>  pattern{ std::nullopt };
};

struct Style
{
  std::vector<Blur>                   blurs;
  std::vector<Border>                 borders;
  std::vector<Fill>                   fills;
  std::vector<Shadow>                 shadows;
  std::optional<std::array<float, 4>> frameRadius;
  float                               cornerSmooth;
};

struct TextLineAttr
{
  bool firstLine{ false };
  int  level{ 0 };
  int  lineType{ TLT_Plain };
};

struct TextStyleAttr
{
  std::string              fontName;
  std::string              subFamilyName;
  std::vector<Fill>        fills;
  float                    letterSpacing{ 0.0 };
  float                    lineSpace{ 0.f };
  float                    baselineShift{ 0.0 };
  size_t                   length{ 0 };
  int                      size{ 14 };
  int                      fillUseType{ 0 };
  float                    fontWeight{ 100 };
  float                    fontWidth{ 100 };
  bool                     bold{ false };
  bool                     italic{ false };
  bool                     lineThrough{ false };
  bool                     kerning{ false };
  ETextUnderline           underline{ UT_None };
  ETextHorizontalAlignment horzAlignment{ HA_Left };
  ELetterTransform         letterTransform{ ELT_Nothing };

  bool operator==(const TextStyleAttr& other) const
  {
    size_t h = 0;
    hash_combine(
      h,
      fontName,
      subFamilyName,
      letterSpacing,
      length,
      size,
      bold,
      italic,
      lineThrough,
      underline,
      horzAlignment);
    return h;
  }
};

struct ControlPoint
{
  glm::vec2                point;
  std::optional<glm::vec2> from;
  std::optional<glm::vec2> to;
  std::optional<int>       cornerStyle;
  float                    radius = 0.0;

  ControlPoint(
    glm::vec2                point,
    float                    radius,
    std::optional<glm::vec2> from,
    std::optional<glm::vec2> to,
    std::optional<int>       cornerStyle)
    : point(point)
    , from(from)
    , to(to)
    , cornerStyle(cornerStyle)
    , radius(radius)
  {
  }
};

struct Contour : public std::vector<ControlPoint>
{
  bool    closed = true;
  float   cornerSmooth{ 0.f };
  EBoolOp blop;
};

using ContourPtr = std::shared_ptr<Contour>;

} // namespace VGG
