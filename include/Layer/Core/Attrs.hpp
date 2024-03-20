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

#include <include/core/SkMatrix.h>
#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <algorithm>
#include <vector>
#include <stdint.h>
#include <variant>
#include <string_view>

namespace VGG
{

struct ContextSetting
{
  EBlendMode    blendMode{ BM_NORMAL };
  float         opacity{ 1.0 };
  bool          isolateBlending{ false };
  EKnockoutType transparencyKnockoutGroup{ KT_OFF };
};

struct ImageFilter
{
  float exposure{ 0.f };
  float contrast{ 0.f };
  float saturation{ 0.f };
  float temperature{ 0.f };
  float tint{ 0.f };
  float highlight{ 0.f };
  float shadow{ 0.f };
  float hue{ 0.f };

  bool isDefault() const
  {
    if (
      exposure != 0.f || contrast != 0.f || saturation != 0.f || temperature != 0.f ||
      tint != 0.f || highlight != 0.f || shadow != 0.f || hue != 0.f)
      return false;
    return true;
  }
};

struct PatternTile
{
  ImageFilter      imageFilter;
  std::string      guid;
  ETilePatternType mode{ TILE_BOTH };
  bool             mirror{ true };
  float            rotation{ 0.f };
  float            scale{ 1.f };
};

struct PatternStretch
{
  ImageFilter      imageFilter;
  std::string      guid;
  layer::Transform transform;
  bool             clip{ false };
};

struct PatternFill
{
  ImageFilter imageFilter;
  std::string guid;
  float       rotation;
};

struct PatternFit
{
  ImageFilter imageFilter;
  std::string guid;
  float       rotation;
};

struct Pattern
{
  std::variant<PatternFill, PatternFit, PatternStretch, PatternTile> instance;
};

struct AlphaMask
{
  std::string    id;
  EAlphaMaskType type{ AM_ALPHA };
  bool           crop{ true };
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

using FillType = std::variant<Gradient, Pattern, Color>;

struct Fill
{
  bool           isEnabled{ true }; // TODO:: Removed
  ContextSetting contextSettings{};
  FillType       type;
};

struct Border
{
  FillType           type;
  ContextSetting     contextSettings;
  std::vector<float> dashedPattern;
  float              dashedOffset;
  float              flat;
  float              miterLimit;
  float              thickness;
  ELineCap           lineCapStyle;
  ELineJoin          lineJoinStyle;
  EPathPosition      position;
  bool               isEnabled;
};

struct Shadow
{
  /*Deprecated: Replaced by ShadowStyle in the future*/
  ContextSetting contextSettings;
  float          blur{ 0.f };
  Color          color;
  float          offsetX{ 0.f };
  float          offsetY{ 0.f };
  float          spread{ 0.f };
  bool           inner{ false };
  bool           isEnabled{ false };
};

struct InnerShadow
{
  ContextSetting contextSettings;
  float          blur{ 0.f };
  Color          color;
  float          offsetX{ 0.f };
  float          offsetY{ 0.f };
  float          spread{ 0.f };
  bool           isEnabled{ false };
};

struct DropShadow
{
  ContextSetting contextSettings;
  float          blur{ 0.f };
  Color          color;
  float          offsetX{ 0.f };
  float          offsetY{ 0.f };
  float          spread{ 0.f };
  bool           isEnabled{ false };
  bool           clipShadow{ true };
};

struct BackgroundBlur
{
  float radius;
};

struct LayerBlur
{
  float radius;
};

struct MotionBlur
{
  float radius;
  float angle;
};

struct RadialBlur
{
  float radius;
  float xCenter, yCenter;
};

using BlurType = std::variant<BackgroundBlur, LayerBlur, MotionBlur, RadialBlur>;

struct Blur
{
  bool     isEnabled;
  BlurType type;
};

struct Style
{
  std::vector<Blur>        blurs;
  std::vector<Border>      borders;
  std::vector<Fill>        fills;
  std::vector<InnerShadow> innerShadow;
  std::vector<DropShadow>  dropShadow;
  std::array<float, 4>     frameRadius;
  float                    cornerSmooth;
};

struct TextLineAttr
{
  bool firstLine{ false };
  int  level{ 0 };
  int  lineType{ TLT_PLAIN };
};

using namespace std::string_view_literals;
struct Font
{
  struct Axis
  {
    uint32_t name;
    float    value;
    Axis(uint32_t tag = 0, float v = 0.f)
      : name(tag)
      , value(v)
    {
    }
  };
  std::string       fontName;
  std::string       subFamilyName;
  std::string       psName;
  std::vector<Axis> axis;
  float             size{ 14 };

  static std::optional<float> axisValue(const std::vector<Axis>& axis, uint32_t tag)
  {
    for (const auto& t : axis)
    {
      if (t.name == tag)
        return t.value;
    }
    return std::nullopt;
  }

  static uint32_t toUint32(const char* tags)
  {
    uint32_t value = ((uint32_t)tags[0] << 24) + ((uint32_t)tags[1] << 16) +
                     ((uint32_t)tags[2] << 8) + (uint32_t)tags[3];
    return value;
  }

  // NOLINTBEGIN
  static constexpr uint32_t wdth =
    (uint32_t('w') << 24) + (uint32_t('d') << 16) + (uint32_t('t') << 8) + uint32_t('h');
  static constexpr uint32_t slnt =
    (uint32_t('s') << 24) + (uint32_t('l') << 16) + (uint32_t('n') << 8) + uint32_t('t');
  static constexpr uint32_t wght =
    (uint32_t('w') << 24) + (uint32_t('g') << 16) + (uint32_t('h') << 8) + uint32_t('t');
  // NOLINTEND
};

struct TextStyleAttr
{
  Font                 font;
  std::vector<Fill>    fills;
  size_t               length{ 0 };
  float                letterSpacing{ 0.0 };
  std::optional<float> lineHeight;
  float                baselineShift{ 0.0 };
  bool                 lineThrough{ false };
  bool                 kerning{ false };
  ETextUnderline       underline{ UT_NONE };
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
