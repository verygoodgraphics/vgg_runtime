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

#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VColor.hpp"
#include "Math/Hash.hpp"
#include "Math/Math.hpp"
#include "Layer/Core/VBounds.hpp"
#include "Layer/Core/VType.hpp"
#include "Utility/Log.hpp"

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

  bool operator==(const ContextSetting& rhs) const
  {
    return blendMode == rhs.blendMode && opacity == rhs.opacity &&
           isolateBlending == rhs.isolateBlending &&
           transparencyKnockoutGroup == rhs.transparencyKnockoutGroup;
  }
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

  bool operator==(const ImageFilter& rhs) const
  {
    return exposure == rhs.exposure && contrast == rhs.contrast && saturation == rhs.saturation &&
           temperature == rhs.temperature && tint == rhs.tint && highlight == rhs.highlight &&
           shadow == rhs.shadow && hue == rhs.hue;
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

  bool operator==(const PatternTile& rhs) const
  {
    return imageFilter == rhs.imageFilter && guid == rhs.guid && mode == rhs.mode &&
           mirror == rhs.mirror && rotation == rhs.rotation && scale == rhs.scale;
  }
};

struct PatternStretch
{
  ImageFilter      imageFilter;
  std::string      guid;
  layer::Transform transform;
  bool             clip{ false };

  bool operator==(const PatternStretch& rhs) const
  {
    return imageFilter == rhs.imageFilter && guid == rhs.guid && transform == rhs.transform &&
           clip == rhs.clip;
  }
};

struct PatternFill
{
  ImageFilter imageFilter;
  std::string guid;
  float       rotation;

  bool operator==(const PatternFill& rhs) const
  {
    return imageFilter == rhs.imageFilter && guid == rhs.guid && rotation == rhs.rotation;
  }
};

struct PatternFit
{
  ImageFilter imageFilter;
  std::string guid;
  float       rotation;

  bool operator==(const PatternFit& rhs) const
  {
    return imageFilter == rhs.imageFilter && guid == rhs.guid && rotation == rhs.rotation;
  }
};

struct Pattern
{
  std::variant<PatternFill, PatternFit, PatternStretch, PatternTile> instance;
  bool operator==(const Pattern& rhs) const
  {
    return instance == rhs.instance;
  }
};

struct AlphaMask
{
  std::string    id;
  EAlphaMaskType type{ AM_ALPHA };
  bool           crop{ true };

  bool operator==(const AlphaMask& rhs) const
  {
    return id == rhs.id && type == rhs.type && crop == rhs.crop;
  }
};

struct GradientStop
{
  Color color{ 1., 1., 1., 1. };
  float position{ 1.0 }; // [0,1]
  float midPoint;

  bool operator==(const GradientStop& rhs) const
  {
    return color == rhs.color && position == rhs.position && midPoint == rhs.midPoint;
  }
};

struct GradientLinear
{
  glm::vec2                 from;
  glm::vec2                 to;
  std::vector<GradientStop> stops;
  bool                      invert{ false };

  bool operator==(const GradientLinear& rhs) const
  {
    return from == rhs.from && to == rhs.to && stops == rhs.stops && invert == rhs.invert;
  }
};

struct GradientRadial
{
  glm::vec2                      from;
  glm::vec2                      to;
  std::vector<GradientStop>      stops;
  std::variant<float, glm::vec2> ellipse;
  bool                           invert{ false };

  bool operator==(const GradientRadial& rhs) const
  {
    return from == rhs.from && to == rhs.to && stops == rhs.stops && ellipse == rhs.ellipse &&
           invert == rhs.invert;
  }
};

struct GradientDiamond
{
  glm::vec2                      from;
  glm::vec2                      to;
  std::vector<GradientStop>      stops;
  std::variant<float, glm::vec2> ellipse;
  bool                           invert{ false };

  bool operator==(const GradientDiamond& rhs) const
  {
    return from == rhs.from && to == rhs.to && stops == rhs.stops && ellipse == rhs.ellipse &&
           invert == rhs.invert;
  }
};

struct GradientAngular
{
  glm::vec2                      from;
  glm::vec2                      to;
  std::vector<GradientStop>      stops;
  std::variant<float, glm::vec2> ellipse;
  bool                           invert{ false };

  bool operator==(const GradientAngular& rhs) const
  {
    return from == rhs.from && to == rhs.to && stops == rhs.stops && ellipse == rhs.ellipse &&
           invert == rhs.invert;
  }
};

struct Gradient
{
  std::variant<GradientLinear, GradientRadial, GradientAngular, GradientDiamond> instance;

  bool operator==(const Gradient& rhs) const
  {
    return instance == rhs.instance;
  }
};

using FillType = std::variant<Gradient, Pattern, Color>;

struct Fill
{
  bool           isEnabled{ true }; // TODO:: Removed
  ContextSetting contextSettings{};
  FillType       type;

  bool operator==(const Fill& rhs) const
  {
    return isEnabled == rhs.isEnabled && contextSettings == rhs.contextSettings && type == rhs.type;
  }
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

  bool operator==(const Border& rhs) const
  {
    return type == rhs.type && contextSettings == rhs.contextSettings &&
           dashedPattern == rhs.dashedPattern && dashedOffset == rhs.dashedOffset &&
           flat == rhs.flat && miterLimit == rhs.miterLimit && thickness == rhs.thickness &&
           lineCapStyle == rhs.lineCapStyle && lineJoinStyle == rhs.lineJoinStyle &&
           position == rhs.position && isEnabled == rhs.isEnabled;
  }
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

  bool operator==(const InnerShadow& rhs) const
  {
    return contextSettings == rhs.contextSettings && blur == rhs.blur && color == rhs.color &&
           offsetX == rhs.offsetX && offsetY == rhs.offsetY && spread == rhs.spread &&
           isEnabled == rhs.isEnabled;
  }
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

  bool operator==(const DropShadow& rhs) const
  {
    return contextSettings == rhs.contextSettings && blur == rhs.blur && color == rhs.color &&
           offsetX == rhs.offsetX && offsetY == rhs.offsetY && spread == rhs.spread &&
           isEnabled == rhs.isEnabled && clipShadow == rhs.clipShadow;
  }
};

struct GaussianBlur
{
  float radius;
  bool  operator==(const GaussianBlur& rhs) const
  {
    return radius == rhs.radius;
  }
};

// struct LayerBlur
// {
//   float radius;
//   bool  operator==(const LayerBlur& rhs) const
//   {
//     return radius == rhs.radius;
//   }
// };

struct MotionBlur
{
  float radius;
  float angle;
  bool  operator==(const MotionBlur& rhs) const
  {
    return radius == rhs.radius && angle == rhs.angle;
  }
};

struct RadialBlur
{
  float radius;
  float xCenter, yCenter;
  bool  operator==(const RadialBlur& rhs) const
  {
    return radius == rhs.radius && xCenter == rhs.xCenter && yCenter == rhs.yCenter;
  }
};

using BlurType = std::variant<GaussianBlur, MotionBlur, RadialBlur>;

struct LayerFX
{
  bool     isEnabled;
  BlurType type;

  LayerFX(bool enabled, BlurType t)
    : isEnabled(enabled)
    , type(t){};

  bool operator==(const LayerFX& rhs) const
  {
    return isEnabled == rhs.isEnabled && type == rhs.type;
  }
};

struct BackgroundFX
{
  bool         isEnabled;
  GaussianBlur blur;
  BackgroundFX(bool enabled, GaussianBlur b)
    : isEnabled(enabled)
    , blur{ b } {};
  bool operator==(const BackgroundFX& rhs) const
  {
    return isEnabled == rhs.isEnabled && blur == rhs.blur;
  }
};

struct Style
{
  std::vector<LayerFX>      layerEffects;
  std::vector<BackgroundFX> backgroundEffects;
  std::vector<Border>       borders;
  std::vector<Fill>         fills;
  std::vector<InnerShadow>  innerShadow;
  std::vector<DropShadow>   dropShadow;

  bool operator==(const Style& rhs) const
  {
    return layerEffects == rhs.layerEffects && borders == rhs.borders && fills == rhs.fills &&
           innerShadow == rhs.innerShadow && dropShadow == rhs.dropShadow;
  }
};

struct TextLineAttr
{
  bool firstLine{ false };
  int  level{ 0 };
  int  lineType{ TLT_PLAIN };

  bool operator==(const TextLineAttr& rhs) const
  {
    return firstLine == rhs.firstLine && level == rhs.level && lineType == rhs.lineType;
  }
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

  bool operator==(const TextStyleAttr& rhs) const
  {
    return font.fontName == rhs.font.fontName && fills == rhs.fills && length == rhs.length &&
           letterSpacing == rhs.letterSpacing && lineHeight == rhs.lineHeight &&
           baselineShift == rhs.baselineShift && lineThrough == rhs.lineThrough &&
           kerning == rhs.kerning && underline == rhs.underline;
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

struct BezierContour : public std::vector<ControlPoint>
{
  bool    closed = true;
  float   cornerSmooth{ 0.f };
  EBoolOp blop{ BO_NONE };
  BezierContour(int reseved)
  {
    reserve(reseved);
  }
};

using ContourPtr = std::shared_ptr<BezierContour>;

} // namespace VGG
