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

#include "Layer/Core/Attrs.hpp"
#include "Domain/Model/DesignModel.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Model/ModelSerde.hpp"
#include "Layer/Core/VBounds.hpp"

#include <ranges>
#include <variant>
#include <vector>

namespace VGG::layer::serde
{
// NOLINTBEGIN

inline void serde_from(const std::vector<double>& v, glm::mat3& x) // NOLINT
{
  if (v.size() != 6)
    return;
  x =
    glm::mat3{ glm::vec3{ v[0], v[1], 0 }, glm::vec3{ v[2], v[3], 0 }, glm::vec3{ v[4], v[5], 1 } };
}
inline void serde_from(const Model::GraphicsContextSettings& t, ContextSetting& x)
{
  x.blendMode = (EBlendMode)t.blendMode;
  x.opacity = t.opacity;
  x.isolateBlending = t.isolateBlending;
  x.transparencyKnockoutGroup = (EKnockoutType)t.transparencyKnockoutGroup;
}

inline void serde_from(const Model::ImageFilters& t, ImageFilter& x) // NOLINT
{
  x.exposure = t.exposure.value_or(0.0);
  x.contrast = t.contrast.value_or(0.0);
  x.saturation = t.saturation.value_or(0.0);
  x.temperature = t.temperature.value_or(0.0);
  x.tint = t.tint.value_or(0.0);
  x.highlight = t.highlights.value_or(0.0);
  x.shadow = t.shadows.value_or(0.0);
  x.hue = t.hue.value_or(0.0);
}

inline void serde_from(const Model::Color& c, Color& x)
{
  x.r = c.red;
  x.g = c.green;
  x.b = c.blue;
  x.a = c.alpha;
}
inline void serde_from(const Model::GradientStop& t, GradientStop& x)
{
  serde::serde_from(t.color, x.color);
  x.position = t.position;
  x.midPoint = t.midPoint;
}

inline void serde_from(const std::vector<Model::GradientStop>& t, std::vector<GradientStop>& x)
{
  for (const auto& s : t)
  {
    x.push_back(GradientStop());
    serde::serde_from(s, x.back());
  }
}

inline void serde_ellipse(
  const std::optional<std::variant<std::vector<double>, double>>& t,
  std::variant<float, glm::vec2>&                                 x)
{
  if (t)
  {
    if (std::holds_alternative<std::vector<double>>(*t))
    {
      auto& v = std::get<std::vector<double>>(*t);
      if (v.size() == 2)
      {
        x = glm::vec2{ v[0], v[1] };
      }
    }
    else if (std::holds_alternative<double>(*t))
    {
      x = (float)std::get<double>(*t);
    }
  }
  x = 1.f; // default value
}

inline void serde_from(const Model::Gradient& t, VGG::Gradient& x)
{
  using namespace VGG::Model;
  glm::vec2   from = { 0.0, 0.0 };
  glm::vec2   to = { 0.0, 0.0 };
  const auto& g = t.instance;
  if (g.from.size() == 2)
  {
    from.x = t.instance.from[0];
    from.y = t.instance.from[1];
  }
  if (g.to.size() == 2)
  {
    to.x = t.instance.to[0];
    to.y = t.instance.to[1];
  }

  switch (t.instance.gradientClass)
  {
    case GradientInstanceClass::GRADIENT_ANGULAR:
    {
      GradientAngular angular;
      angular.from = from;
      angular.to = to;
      serde::serde_from(g.stops, angular.stops);
      serde::serde_ellipse(g.ellipse, angular.ellipse);
      x.instance = angular;
    }
    break;
    case GradientInstanceClass::GRADIENT_DIAMOND:
    {
      GradientDiamond diamond;
      diamond.from = from;
      diamond.to = to;
      serde::serde_from(g.stops, diamond.stops);
      serde::serde_ellipse(g.ellipse, diamond.ellipse);
      x.instance = diamond;
    }
    break;
    case GradientInstanceClass::GRADIENT_LINEAR:
    {
      GradientLinear linear;
      linear.from = from;
      linear.to = to;
      serde::serde_from(g.stops, linear.stops);
      x.instance = linear;
    }
    break;
    case GradientInstanceClass::GRADIENT_RADIAL:
    {
      GradientRadial radial;
      radial.from = from;
      radial.to = to;
      serde::serde_from(g.stops, radial.stops);
      serde::serde_ellipse(g.ellipse, radial.ellipse);
      x.instance = radial;
    }
    case GradientInstanceClass::GRADIENT_BASIC:
    {
      DEBUG("not support");
    }
    break;
  }
}

inline void serde_from(const Model::Pattern& t, VGG::Pattern& x)
{
  using namespace VGG::Model;
  if (auto p = std::get_if<PatternImageFill>(&t.instance); p)
  {
    PatternFill f;
    if (p->imageFilters)
    {
      serde::serde_from(*p->imageFilters, f.imageFilter);
    }
    f.guid = p->imageFileName;
    f.rotation = p->rotation;
    x.instance = f;
  }
  else if (auto p = std::get_if<PatternImageFit>(&t.instance); p)
  {
    PatternFit f;
    if (p->imageFilters)
    {
      serde::serde_from(*p->imageFilters, f.imageFilter);
    }
    f.guid = p->imageFileName;
    f.rotation = p->rotation;
    x.instance = f;
  }
  else if (auto p = std::get_if<PatternImageStrech>(&t.instance); p)
  {
    PatternStretch f;
    if (p->imageFilters)
    {
      serde::serde_from(*p->imageFilters, f.imageFilter);
    }
    f.guid = p->imageFileName;
    layer::Transform transform;
    glm::mat3        m;
    serde::serde_from(p->matrix, m);
    f.transform = layer::Transform(m);
    x.instance = f;
  }
  else if (auto p = std::get_if<PatternImageTile>(&t.instance); p)
  {
    PatternTile f;

    if (p->imageFilters)
    {
      serde::serde_from(*p->imageFilters, f.imageFilter);
    }
    f.guid = p->imageFileName;
    f.rotation = p->rotation;
    f.mirror = p->mirror.value_or(false);
    f.scale = p->scale;
    f.mode = (ETilePatternType)p->mode.value_or(int(TILE_BOTH));
    x.instance = f;
  }
  else if (auto p = std::get_if<PatternLayerInstance>(&t.instance); p)
  {
  }
}

inline void serde_from(const Model::Border& b, FillType& x)
{
  switch ((EPathFillType)b.fillType)
  {
    case FT_COLOR:
    {
      if (b.color)
      {
        Color c;
        serde::serde_from(*b.color, c);
        x = c;
      }
      break;
    }
    case FT_PATTERN:
      if (b.pattern)
      {
        Pattern p;
        serde::serde_from(*b.pattern, p);
        x = p;
        break;
      }
    case FT_GRADIENT:
      if (b.gradient)
      {
        Gradient g;
        serde::serde_from(*b.gradient, g);
        x = g;
        break;
      }
    default:
      break;
  }
}

inline void serde_from(const Model::Fill& b, FillType& x)
{
  switch ((EPathFillType)b.fillType)
  {
    case FT_COLOR:
    {
      if (b.color)
      {
        Color c;
        serde::serde_from(*b.color, c);
        x = c;
      }
      break;
    }
    case FT_PATTERN:
      if (b.pattern)
      {
        Pattern p;
        serde::serde_from(*b.pattern, p);
        x = p;
        break;
      }
    case FT_GRADIENT:
      if (b.gradient)
      {
        Gradient g;
        serde::serde_from(*b.gradient, g);
        x = g;
        break;
      }
    default:
      break;
  }
}

inline void serde_from(const Model::Border& b, Border& x)
{
  x.isEnabled = b.isEnabled;
  serde::serde_from(b.contextSettings, x.contextSettings);
  serde::serde_from(b, x.type);

  x.dashedOffset = b.dashedOffset;
  x.dashedPattern = std::vector<float>(b.dashedPattern.begin(), b.dashedPattern.end());
  if (x.dashedPattern.size() % 2 != 0)
  {
    x.dashedPattern.insert(x.dashedPattern.end(), x.dashedPattern.begin(), x.dashedPattern.end());
  }
  x.flat = b.flat;
  x.lineCapStyle = (ELineCap)b.lineCapStyle;
  x.lineJoinStyle = (ELineJoin)b.lineJoinStyle;
  x.miterLimit = b.miterLimit;
  x.position = (EPathPosition)b.position;
  x.thickness = b.thickness;
}

inline void serde_from(const Model::Fill& b, Fill& x)
{
  x.isEnabled = b.isEnabled;
  serde::serde_from(b.contextSettings, x.contextSettings);
  serde::serde_from(b, x.type);
}

inline void serde_from(const Model::FontVariation& t, Font::Axis& x)
{
  if (t.name.size() == 4)
  {
    x.name = Font::toUint32(t.name.c_str());
    x.value = t.value;
  }
}

inline void serde_from(const Model::Shadow& t, InnerShadow& x)
{
  x.isEnabled = t.isEnabled;
  x.blur = t.blur;
  serde::serde_from(t.color, x.color);
  serde::serde_from(t.contextSettings, x.contextSettings);
  x.offsetX = t.offsetX;
  x.offsetY = t.offsetY;
  x.spread = t.spread;
}

inline void serde_from(const Model::Shadow& t, DropShadow& x)
{
  x.isEnabled = t.isEnabled;
  x.blur = t.blur;
  serde::serde_from(t.color, x.color);
  serde::serde_from(t.contextSettings, x.contextSettings);
  x.offsetX = t.offsetX;
  x.offsetY = t.offsetY;
  x.spread = t.spread;
  x.clipShadow = !t.showBehindTransparentAreas.value_or(false);
}

template<>
struct ModelSerde<VGG::Model::Rect, Bounds>
{
  static Bounds serde_from(const Model::Rect& t) // NOLINT
  {
    Bounds x{ float(t.x), float(t.y), float(t.width), float(t.height) };
    return x;
  }
};

template<>
struct ModelSerde<std::vector<double>, glm::mat3>
{
  static glm::mat3 serde_from(const std::vector<double>& v) // NOLINT
  {
    glm::mat3 x;
    serde::serde_from(v, x);
    return x;
  }
};

template<>
struct ModelSerde<Model::Style, VGG::Style>
{
  static Style serde_from(const Model::Style& t) // NOLINT
  {
    Style x;
    x.layerEffects.clear();
    x.backgroundEffects.clear();
    for (const auto& b : t.blurs)
    {
      auto        isEnabled = b.isEnabled;
      const auto  blurType = (EBlurType)b.type;
      const float radius = b.radius.value_or(0.f);
      switch (blurType)
      {
        case BT_LAYER:
          x.layerEffects.emplace_back(isEnabled, GaussianBlur{ radius });
          break;
        case BT_BACKGROUND:
          x.backgroundEffects.emplace_back(isEnabled, GaussianBlur{ radius });
          break;
        case BT_RADIAL:
        {
          glm::vec2 center = { 0, 0 };
          if (b.center.size() == 2)
          {
            center.x = b.center[0];
            center.y = b.center[1];
          }
          x.layerEffects.emplace_back(isEnabled, RadialBlur{ radius, center.x, center.y });
          break;
        }
        case BT_MOTION:
          x.layerEffects.emplace_back(
            isEnabled,
            MotionBlur{ radius, (float)b.motionAngle.value_or(0.f) });
          break;
        default:
          break;
      }
    }

    for (const auto& f : t.fills)
    {
      x.fills.push_back(Fill());
      serde::serde_from(f, x.fills.back());
    }

    for (const auto& b : t.borders)
    {
      x.borders.push_back(Border());
      serde::serde_from(b, x.borders.back());
    }

    for (const auto& sh : t.shadows)
    {
      if (sh.inner)
      {
        x.innerShadow.push_back(InnerShadow());
        serde::serde_from(sh, x.innerShadow.back());
      }
      else
      {
        x.dropShadow.push_back(DropShadow());
        serde::serde_from(sh, x.dropShadow.back());
      }
    }
    return x;
  }
};

template<>
struct ModelSerde<Model::GraphicsContextSettings, ContextSetting>
{
  static ContextSetting serde_from(const Model::GraphicsContextSettings& t) // NOLINT
  {
    ContextSetting x;
    serde::serde_from(t, x);
    return x;
  }
};

template<>
struct ModelSerde<Model::AlphaMask, VGG::AlphaMask>
{
  static VGG::AlphaMask serde_from(const Model::AlphaMask& t) // NOLINT
  {
    VGG::AlphaMask x;
    x.id = t.id;
    x.crop = t.crop;
    x.type = (VGG::EAlphaMaskType)t.alphaType;
    return x;
  }
};

template<>
struct ModelSerde<Model::ImageFilters, VGG::ImageFilter>
{
  static VGG::ImageFilter serde_from(const Model::ImageFilters& t) // NOLINT
  {
    VGG::ImageFilter x;
    serde::serde_from(t, x);
    return x;
  }
};

template<>
struct ModelSerde<std::vector<double>, std::optional<std::array<float, 2>>>
{
  static std::optional<std::array<float, 2>> serde_from(const std::vector<double>& v) // NOLINT
  {
    if (v.size() != 2)
      return std::array{ 0.0f, 0.0f };
    return std::array{ (float)v[0], (float)v[1] };
  }
};

template<>
struct ModelSerde<Model::TextLineType, TextLineAttr>
{
  static TextLineAttr serde_from(const Model::TextLineType& t) // NOLINT
  {
    TextLineAttr x;
    x.level = t.level;
    x.lineType = t.styleType;
    x.firstLine = t.isFirst;
    return x;
  }
};

template<>
struct ModelSerde<Model::TextFontAttributes, TextStyleAttr>
{
  static TextStyleAttr serde_from(const Model::TextFontAttributes& t) // NOLINT
  {
    TextStyleAttr x;

    x.length = t.length.value_or(0);
    x.font.fontName = t.name.value_or("");
    x.font.subFamilyName = t.subFamilyName.value_or("");
    x.font.psName = t.postScript.value_or("");
    if (t.fontVariations)
    {
      for (const auto& f : *t.fontVariations)
      {
        x.font.axis.push_back(Font::Axis());
        serde::serde_from(f, x.font.axis.back());
      }
    }
    x.font.size = t.size.value_or(14.0);
    if (t.fills)
    {
      for (const auto& f : *t.fills)
      {
        x.fills.push_back(VGG::Fill());
        serde::serde_from(f, x.fills.back());
      }
    }
    x.baselineShift = t.baselineShift.value_or(0.0);
    x.lineThrough = t.linethrough.value_or(false);
    x.underline = (ETextUnderline)t.underline.value_or((int)UT_NONE);
    // x.kerning = t.kerning
    x.letterSpacing = t.letterSpacingValue.value_or(0.f);
    auto unit = t.letterSpacingUnit.value_or(1);
    if (unit == 1)
    {
      x.letterSpacing = x.font.size * x.letterSpacing / 100.f;
    }
    auto lineSpacingUnit = t.lineSpacingUnit.value_or(0);
    auto lineHeightRaw = t.lineSpacingValue.value_or(0.f);
    if (lineSpacingUnit == 0)
    {
      x.lineHeight = lineHeightRaw / x.font.size;
    }
    else if (lineSpacingUnit == 2)
    {
      x.lineHeight = lineHeightRaw;
    }
    return x;
  }
};

template<>
struct ModelSerde<std::vector<double>, std::array<float, 4>>
{
  static std::array<float, 4> serde_from(const std::vector<double>& t) // NOLINT
  {
    if (t.size() != 4)
      return { 0.0f, 0.0f, 0.0f, 0.0f };
    return { (float)t[0], (float)t[1], (float)t[2], (float)t[3] };
  }
};

// NOLINTEND

} // namespace VGG::layer::serde
