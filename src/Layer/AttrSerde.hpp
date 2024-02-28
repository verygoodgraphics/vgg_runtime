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
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Math/Algebra.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <optional>
using nlohmann::json;

// NOLINTBEGIN
namespace nlohmann
{

template<>
struct adl_serializer<std::variant<float, glm::vec2>>
{
  static void from_json(const json& j, std::variant<float, glm::vec2>& x)
  {
    if (j.is_number())
    {
      x = (float)j;
    }
    else if (j.is_array())
    {
      std::array<float, 2> v = j;
      x = glm::vec2{ v[0], v[1] };
    }
  }
};
} // namespace nlohmann
template<typename T>
inline std::optional<T> getOptional(const nlohmann::json& obj, const std::string& key)
{
  if (auto p = obj.find(key); p != obj.end())
    return p.value().get<T>();
  return std::nullopt;
}

template<typename T>
inline std::optional<T> getOptional(const nlohmann::json& obj, const char* key)
{
  if (auto p = obj.find(key); p != obj.end())
    return p.value().get<T>();
  return std::nullopt;
}

template<typename K>
inline const nlohmann::json& getOrDefault(const nlohmann::json& j, K&& key)
{
  if (auto it = j.find(key); it != j.end())
  {
    return it.value();
  }

  static const nlohmann::json s_json;
  return s_json;
}

template<>
inline std::optional<glm::vec2> getOptional(const nlohmann::json& obj, const std::string& key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto v = obj.value(key, std::array<float, 2>{ 0, 0 });
    return glm::vec2(v[0], v[1]);
  }
  return std::nullopt;
}

template<>
inline std::optional<glm::vec2> getOptional(const nlohmann::json& obj, const char* key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto v = obj.value(key, std::array<float, 2>{ 0, 0 });
    return glm::vec2(v[0], v[1]);
  }
  return std::nullopt;
}

namespace VGG
{

template<typename T>
inline std::optional<T> getStackOptional(const json& j, const char* property)
{
  auto it = j.find(property);
  if (it != j.end() && !it->is_null())
  {
    return std::optional<T>(it.value());
  }
  return std::optional<T>();
}

template<typename T>
inline std::optional<T> getStackOptional(const json& j, const std::string& property)
{
  return getStackOptional<T>(j, property.data());
}

inline void from_json(const json& j, Color& x)
{
  if (!j.is_object())
    return;
  x.a = j.value("alpha", 0.f);
  x.b = j.value("blue", 0.f);
  x.g = j.value("green", 0.f);
  x.r = j.value("red", 0.f);
}

inline void from_json(const json& j, glm::vec2& x)
{
  if (j.is_array())
  {
    std::array<float, 2> v = j.get<std::array<float, 2>>();
    x = { v[0], v[1] };
  }
  x = { 0.f, 0.f };
}

inline void from_json(const json& j, std::variant<float, glm::vec2>& x)
{
  if (j.is_number())
  {
    x = (float)j;
  }
  else if (j.is_array())
  {
    std::array<float, 2> v = j.get<std::array<float, 2>>();
    x = glm::vec2{ v[0], v[1] };
  }
}

inline void from_json(const json& j, ImageFilter& x)
{
  if (!j.is_object())
    return;
  x.exposure = j.value("exposure", 0.f);
  x.contrast = j.value("contrast", 0.f);
  x.saturation = j.value("saturation", 0.f);
  x.temperature = j.value("temperature", 0.f);
  x.tint = j.value("tint", 0.f);
  x.highlight = j.value("highlights", 0.f);
  x.shadow = j.value("shadows", 0.f);
  x.hue = j.value("hue", 0.f);
}

inline void from_json(const json& j, PatternFill& x)
{
  if (!j.is_object())
    return;
  x.imageFilter = j.value("imageFilters", ImageFilter{});
  x.guid = j.value("imageFileName", "");
  x.rotation = glm::radians(j.value("rotation", 0.f));
}

inline void from_json(const json& j, PatternFit& x)
{
  if (!j.is_object())
    return;
  x.imageFilter = j.value("imageFilters", ImageFilter{});
  x.guid = j.value("imageFileName", "");
  x.rotation = glm::radians(j.value("rotation", 0.f));
}

inline void from_json(const json& j, PatternStretch& x)
{
  if (!j.is_object())
    return;
  x.imageFilter = j.value("imageFilters", ImageFilter{});
  x.guid = j.value("imageFileName", "");
  x.clip = j.value("crop", true);
  layer::Transform transform;
  auto             v = j.value("matrix", std::array<float, 6>());
  auto             m =
    glm::mat3{ glm::vec3{ v[0], v[1], 0 }, glm::vec3{ v[2], v[3], 0 }, glm::vec3{ v[4], v[5], 1 } };
  x.transform = layer::Transform(m);
}

inline void from_json(const json& j, PatternTile& x)
{
  if (!j.is_object())
    return;
  x.imageFilter = j.value("imageFilters", ImageFilter{});
  x.guid = j.value("imageFileName", "");
  x.mirror = j.value("mirror", false);
  x.rotation = glm::radians(j.value("rotation", 0.f));
  x.scale = j.value("scale", 1.f);
  x.mode = j.value("mode", TILE_BOTH);
}

inline void from_json(const json& j, Pattern& x)
{
  if (!j.is_object())
    return;
  auto       instance = j.value("instance", json{});
  const auto klass = instance.value("class", "");
  if (klass == "patternImageFit")
  {
    x.instance = PatternFit(instance);
  }
  else if (klass == "patternImageFill")
  {
    x.instance = PatternFill(instance);
  }
  else if (klass == "patternImageStretch")
  {
    x.instance = PatternStretch(instance);
  }
  else if (klass == "patternImageTile")
  {
    x.instance = PatternTile(instance);
  }
  else if (klass == "patternLayer")
  {
    // TODO:: Feature for patternLayer
  }
  else
  {
  }
}

inline void from_json(const json& j, GradientStop& x)
{
  if (!j.is_object())
    return;
  x.color = j["color"];
  x.position = j["position"];
  x.midPoint = j["midPoint"];
}

inline void from_json(const json& j, Gradient& x)
{
  if (!j.is_object())
    return;
  const auto g = j["instance"];
  const auto klass = g["class"];
  if (klass == "gradientLinear")
  {
    const auto     f = g.value("from", std::array<float, 2>{ 0, 0 });
    const auto     t = g.value("to", std::array<float, 2>{ 0, 0 });
    GradientLinear linear;
    linear.from = glm::vec2{ f[0], f[1] };
    linear.to = glm::vec2{ t[0], t[1] };
    linear.invert = g.value("invert", false);
    linear.stops = g.value("stops", std::vector<GradientStop>());
    x.instance = linear;
  }
  else if (klass == "gradientRadial")
  {
    const auto     f = g.value("from", std::array<float, 2>{ 0, 0 });
    const auto     t = g.value("to", std::array<float, 2>{ 0, 0 });
    GradientRadial radial;
    radial.from = glm::vec2{ f[0], f[1] };
    radial.to = glm::vec2{ t[0], t[1] };
    radial.stops = g.value("stops", std::vector<GradientStop>());
    radial.invert = g.value("invert", false);
    radial.ellipse = g.value("ellipse", std::variant<float, glm::vec2>(1.f));
    x.instance = radial;
  }
  else if (klass == "gradientAngular")
  {
    const auto      f = g.value("from", std::array<float, 2>{ 0, 0 });
    const auto      t = g.value("to", std::array<float, 2>{ 0, 0 });
    GradientAngular angular;
    angular.from = glm::vec2{ f[0], f[1] };
    angular.to = glm::vec2{ t[0], t[1] };
    angular.stops = g.value("stops", std::vector<GradientStop>());
    angular.invert = g.value("invert", false);
    angular.ellipse = g.value("ellipse", std::variant<float, glm::vec2>(1.f));
    x.instance = angular;
  }
  else if (klass == "gradientDiamond")
  {
    const auto      f = g.value("from", std::array<float, 2>{ 0, 0 });
    const auto      t = g.value("to", std::array<float, 2>{ 0, 0 });
    GradientDiamond diamond;
    diamond.from = glm::vec2{ f[0], f[1] };
    diamond.to = glm::vec2{ t[0], t[1] };
    diamond.stops = g.value("stops", std::vector<GradientStop>());
    diamond.invert = g.value("invert", false);
    diamond.ellipse = g.value("ellipse", std::variant<float, glm::vec2>(1.f));
    x.instance = diamond;
  }
  else if (klass == "gradientBasic")
  {
    GradientBasic basic;
    x.instance = basic;
  }
}

inline void from_json(const json& j, ContextSetting& x)
{
  if (!j.is_object())
    return;
  x.blendMode = j.value("blendMode", EBlendMode::BM_PASS_THROUGHT);
  x.isolateBlending = j.value("isolateBlending", false);
  x.opacity = j.value("opacity", 1.f);
  x.transparencyKnockoutGroup = j.value("transparencyKnockoutGroup", EKnockoutType{});
}

///
/// json object j must have key:
/// "fillType"
/// "color"
/// "pattern"
/// "gradient"
///
inline FillType makeFillType(const nlohmann::json& j)
{
  FillType type;
  switch (j.value("fillType", EPathFillType{}))
  {
    case FT_COLOR:
      type = j.value("color", Color{});
      break;
    case FT_PATTERN:
      type = j.value("pattern", Pattern{});
      break;
    case FT_GRADIENT:
      type = j.value("gradient", Gradient{});
      break;
    default:
      break;
  }
  return type;
}

inline void from_json(const json& j, Border& x)
{
  // x.color = get_stack_optional<Color>(j, "color");
  // x.fillType = j.at("fillType").get<EPathFillType>();
  // x.gradient = get_stack_optional<Gradient>(j, "gradient");
  // x.pattern = get_stack_optional<Pattern>(j, "pattern");
  if (!j.is_object())
    return;
  x.type = makeFillType(j);
  x.contextSettings = j.value("contextSettings", ContextSetting());
  x.dashedOffset = j.value("dashedOffset", 0.f);
  x.dashedPattern = j.value("dashedPattern", std::vector<float>());
  if (x.dashedPattern.size() % 2 != 0)
  {
    x.dashedPattern.insert(x.dashedPattern.end(), x.dashedPattern.begin(), x.dashedPattern.end());
  }
  x.flat = j.value("flat", 0.f);
  x.isEnabled = j.value("isEnabled", false);
  x.lineCapStyle = j.value("lineCapStyle", ELineCap::LC_BUTT);
  x.lineJoinStyle = j.value("lineJoinStyle", ELineJoin::LJ_MITER);
  x.miterLimit = j.value("miterLimit", 0.f);
  x.position = j.value("position", EPathPosition::PP_CENTER);
  x.thickness = j.value("thickness", 0.f);
}

inline void from_json(const json& j, Shadow& x)
{
  if (!j.is_object())
    return;
  x.blur = j.value("blur", 0.f);
  x.color = j.value("color", Color());
  x.contextSettings = j.value("contextSettings", ContextSetting());
  x.inner = j.value("inner", false);
  x.isEnabled = j.value("isEnabled", false);
  const auto p = glm::vec2{ j.value("offsetX", 0.f), j.value("offsetY", 0.f) };
  x.offsetX = p.x;
  x.offsetY = p.y;
  x.spread = j.value("spread", 0.f);
}

inline void from_json(const json& j, InnerShadowStyle& x)
{
  x.blur = j.value("blur", 0.f);
  x.color = j.value("color", Color());
  x.contextSettings = j.value("contextSettings", ContextSetting());
  x.isEnabled = j.value("isEnabled", false);
  const auto p = glm::vec2{ j.value("offsetX", 0.f), j.value("offsetY", 0.f) };
  x.offsetX = p.x;
  x.offsetY = p.y;
  x.spread = j.value("spread", 0.f);
}

inline void from_json(const json& j, OuterShadowStyle& x)
{
  x.blur = j.value("blur", 0.f);
  x.color = j.value("color", Color());
  x.contextSettings = j.value("contextSettings", ContextSetting());
  x.isEnabled = j.value("isEnabled", false);
  const auto p = glm::vec2{ j.value("offsetX", 0.f), j.value("offsetY", 0.f) };
  x.offsetX = p.x;
  x.offsetY = p.y;
  x.spread = j.value("spread", 0.f);
  x.clipShadow = !j.value("showBehindTransparentAreas", true);
}

inline void from_json(const json& j, ShadowStyle& x)
{
  bool inner = j.at("inner").get<bool>();
  if (!inner)
    x = (OuterShadowStyle)j;
  else
    x = (InnerShadowStyle)j;
}

inline void from_json(const json& j, Blur& x)
{
  if (!j.is_object())
    return;
  x.radius = getStackOptional<float>(j, "radius").value_or(0.f);
  x.motionAngle = getStackOptional<float>(j, "motionAngle").value_or(0.f);
  const auto v = j.value("center", std::array<float, 2>{ 0, 0 });
  x.center = glm::vec2(v[0], v[1]);
  x.isEnabled = j.value("isEnabled", false);
  x.blurType = j.value("type", EBlurType());
  x.saturation = j.value("saturation", 0.f);
}

inline void from_json(const json& j, Fill& x)
{
  if (!j.is_object())
    return;
  x.isEnabled = j.value("isEnabled", false);
  x.contextSettings = j.value("contextSettings", ContextSetting());
  x.type = makeFillType(j);
  // x.fillType = (EPathFillType)j.at("fillType").get<int>();
  // x.gradient = get_stack_optional<Gradient>(j, "gradient");
  // x.pattern = get_stack_optional<Pattern>(j, "pattern");
  // x.color = j.value("color", Color{});
}

inline void from_json(const json& j, Style& x)
{
  x.blurs = j.value("blurs", std::vector<Blur>());
  x.borders = j.value("borders", std::vector<Border>());
  x.fills = j.value("fills", std::vector<Fill>());
  x.shadows = j.value("shadows", std::vector<Shadow>());
  for (const auto& s : j.value("shadows", json::array_t{}))
  {
    if (s.value("inner", false))
      x.innerShadow.push_back(s);
    else
      x.dropShadow.push_back(s);
  }
  // x.shadowStyle = j.value("shadows", std::vector<ShadowStyle>());
}

inline void from_json(const json& j, TextLineAttr& x)
{
  if (!j.is_object())
    return;
  x.level = j.value("level", 0);
  x.firstLine = j.value("isFirst", false);
  x.lineType = j.value("styleType", 0);
}

inline void from_json(const json& j, Bound& b)
{
  if (!j.is_object())
    return;
  auto       x = j.value("x", 0.f);
  auto       y = j.value("y", 0.f);
  const auto topLeft = glm::vec2{ x, y };
  auto       width = j.value("width", 0.f);
  auto       height = j.value("height", 0.f);
  b = Bound{ topLeft, width, height };
}

inline void from_json(const json& j, Font::Axis& x)
{
  if (!j.is_object())
    return;
  auto tag = j.value("name", "");
  if (tag.size() == 4)
  {
    x.name = Font::toUint32(tag.c_str());
    x.value = j.value("value", 0.f);
  }
}

inline void from_json(const json& j, TextStyleAttr& x)
{
  if (!j.is_object())
    return;
  x.length = j.value("length", 0);
  x.font.fontName = j.value("name", "");
  x.font.subFamilyName = j.value("subFamilyName", "");
  x.font.psName = j.value("postScript", "");
  x.font.axis = j.value("fontVariations", std::vector<Font::Axis>());
  x.font.size = j.value("size", 14.0);
  x.fills = j.value("fills", std::vector<Fill>());
  x.baselineShift = j.value("baselineShift", 0.0);
  x.lineThrough = j.value("linethrough", false);
  x.underline = j.value("underline", UT_NONE);
  x.kerning = j.value("kerning", false);
  x.letterSpacing = j.value("letterSpacingValue", 0.0);
  auto unit = j.value("letterSpacingUnit", 1);
  if (unit == 1)
  {
    x.letterSpacing = x.font.size * x.letterSpacing / 100.f;
  }

  auto lineSpacingUnit = j.value("lineSpacingUnit", 0);
  auto lineHeightRaw = j.value("lineSpacingValue", 0.0);
  if (lineSpacingUnit == 0)
  {
    x.lineHeight = lineHeightRaw / x.font.size;
  }
  else if (lineSpacingUnit == 2)
  {
    x.lineHeight = lineHeightRaw;
  }
}

inline void from_json(const json& j, AlphaMask& x)
{
  if (!j.is_object())
    return;
  x.id = j.value("id", "");
  x.type = j.value("alphaType", AM_ALPHA);
  x.crop = j.value("crop", true);
}
// NOLINTEND
} // namespace VGG
