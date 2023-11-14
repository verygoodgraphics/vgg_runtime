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

namespace VGG
{
constexpr bool FLIP_COORD = false;
} // namespace VGG

// NOLINTBEGIN
template<typename T>
inline std::optional<T> get_opt(const nlohmann::json& obj, const std::string& key)
{
  if (auto p = obj.find(key); p != obj.end())
    return p.value().get<T>();
  return std::nullopt;
}

template<typename T>
inline std::optional<T> get_opt(const nlohmann::json& obj, const char* key)
{
  if (auto p = obj.find(key); p != obj.end())
    return p.value().get<T>();
  return std::nullopt;
}

template<typename K>
inline const nlohmann::json& get_or_default(const nlohmann::json& j, K&& key)
{
  if (auto it = j.find(key); it != j.end())
  {
    return it.value();
  }

  static const nlohmann::json s_json;
  return s_json;
}

template<>
inline std::optional<glm::vec2> get_opt(const nlohmann::json& obj, const std::string& key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto v = obj.value(key, std::array<float, 2>{ 0, 0 });
    return glm::vec2(v[0], v[1]);
  }
  return std::nullopt;
}

template<>
inline std::optional<glm::vec2> get_opt(const nlohmann::json& obj, const char* key)
{
  if (auto it = obj.find(key); it != obj.end())
  {
    auto v = obj.value(key, std::array<float, 2>{ 0, 0 });
    return glm::vec2(v[0], v[1]);
  }
  return std::nullopt;
}

// NOLINTEND

namespace VGG
{
// NOLINTBEGIN

template<typename T>
inline std::optional<T> get_stack_optional(const json& j, const char* property)
{
  auto it = j.find(property);
  if (it != j.end() && !it->is_null())
  {
    return std::optional<T>(it.value());
  }
  return std::optional<T>();
}

template<typename T>
inline std::optional<T> get_stack_optional(const json& j, const std::string& property)
{
  return get_stack_optional<T>(j, property.data());
}

inline void from_json(const json& j, Color& x)
{
  x.a = j["alpha"];
  x.b = j["blue"];
  x.g = j["green"];
  x.r = j["red"];
}

inline void from_json(const json& j, Pattern& x)
{
  const auto instance = j["instance"];
  const auto klass = instance["class"];
  if (klass == "patternImage")
  {
    x.imageFillType = instance["fillType"];
    x.imageGUID = instance["imageFileName"];
    x.tileMirrored = instance["imageTileMirrored"];
    x.tileScale = 1.0;

    const auto it = instance.find("matrix");
    glm::mat3 m{ 1.0 };
    if (it != instance.end())
    {
      auto v = instance.at("matrix").get<std::array<float, 6>>();
      m = glm::mat3{ glm::vec3{ v[0], v[1], 0 },
                     glm::vec3{ v[2], v[3], 0 },
                     glm::vec3{ v[4], v[5], 1 } };
    }
    x.transform = m;
  }
  else if (klass == "patternLayer")
  {
    // TODO:: Feature for patternLayer
  }
}

inline void from_json(const json& j, Gradient::GradientStop& x)
{
  x.color = j["color"];
  x.position = j["position"];
  x.midPoint = j["midPoint"];
}

inline void transformGredient(const json& j, Gradient& x)
{
  x.aiCoordinate = true;
  const float length = j["length"];
  const float angle = j["angle"];
  x.from = glm::vec2(j["xOrigin"], 0);
  x.to.x = angle;
  x.to.y = length;
}

inline void from_json(const json& j, Gradient& x)
{
  const auto g = j["instance"];
  const auto klass = g["class"];
  if (klass != "gradientBasic")
  {
    const auto f = g["from"];
    const auto t = g["to"];
    x.from = glm::vec2{ f[0], f[1] };
    x.to = glm::vec2{ t[0], t[1] };
    x.stops = g["stops"];
    x.invert = g["invert"];
    x.gradientType = EGradientType::GT_Linear;
    if (klass == "gradientRadial")
    {
      x.elipseLength = g["elipseLength"];
      x.elipseLength = x.elipseLength == 0.0 ? 1.0 : x.elipseLength; // TODO::fixup
      x.gradientType = EGradientType::GT_Radial;
    }
    else if (klass == "gradientAngular")
    {
      x.gradientType = EGradientType::GT_Angular;
      for (auto& stop : x.stops)
      {
        stop.position = 1.0 - stop.position;
      }
    }
  }
  else
  {
    x.stops = g["stops"];
    x.gradientType = g["gradientType"];
    auto geo = g["geometry"];
    transformGredient(geo, x);
  }
}

inline void from_json(const json& j, ContextSetting& x)
{
  x.BlendMode = j.at("blendMode").get<EBlendMode>();
  x.IsolateBlending = j.at("isolateBlending").get<bool>();
  x.Opacity = j.at("opacity").get<double>();
  x.TransparencyKnockoutGroup = j.at("transparencyKnockoutGroup").get<EKnockoutType>();
}

inline void from_json(const json& j, Border& x)
{
  x.color = get_stack_optional<Color>(j, "color");
  x.context_settings = j.at("contextSettings").get<ContextSetting>();
  x.dashed_offset = j.at("dashedOffset").get<double>();
  x.dashed_pattern = j.at("dashedPattern").get<std::vector<float>>();
  x.fill_type = j.at("fillType").get<EPathFillType>();
  x.flat = j.at("flat").get<double>();
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  x.isEnabled = j.at("isEnabled").get<bool>();
  x.lineCapStyle = j.at("lineCapStyle").get<ELineCap>();
  x.lineJoinStyle = j.at("lineJoinStyle").get<ELineJoin>();
  x.miterLimit = j.at("miterLimit").get<double>();
  x.pattern = get_stack_optional<Pattern>(j, "pattern");
  x.position = j.at("position").get<EPathPosition>();
  x.style = j.at("style").get<int64_t>();
  x.thickness = j.at("thickness").get<double>();
}

inline void from_json(const json& j, Shadow& x)
{
  x.blur = j.at("blur").get<double>();
  x.color = j.at("color").get<Color>();
  x.context_settings = j.at("contextSettings").get<ContextSetting>();
  x.inner = j.at("inner").get<bool>();
  x.is_enabled = j.at("isEnabled").get<bool>();
  const auto p = glm::vec2{ j.value("offsetX", 0.f), j.value("offsetY", 0.f) };
  x.offset_x = p.x;
  x.offset_y = p.y;
  x.spread = j.at("spread").get<double>();
}

inline void from_json(const json& j, Blur& x)
{
  x.radius = get_stack_optional<float>(j, "radius").value_or(0.f);
  x.motionAngle = get_stack_optional<float>(j, "motionAngle").value_or(0.f);
  const auto v = j.value("center", std::array<float, 2>{ 0, 0 });
  x.center = glm::vec2(v[0], v[1]);
  x.isEnabled = j.value("isEnabled", false);
  x.blurType = j.value("type", EBlurType());
  x.saturation = j.value("saturation", 0.f);
}

inline void from_json(const json& j, Fill& x)
{
  x.color = Color{ 0, 0, 0, 1 };
  if (auto it = j.find("color"); it != j.end())
  {
    x.color = j.at("color").get<Color>();
  }
  x.fillType = (EPathFillType)j.at("fillType").get<int>();
  x.isEnabled = j.at("isEnabled").get<bool>();
  x.contextSettings = j.at("contextSettings").get<ContextSetting>();
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  x.pattern = get_stack_optional<Pattern>(j, "pattern");
}

inline void from_json(const json& j, Style& x)
{
  x.blurs = j.at("blurs").get<std::vector<Blur>>();
  x.borders = j.at("borders").get<std::vector<Border>>();
  x.fills = j.at("fills").get<std::vector<Fill>>();
  x.shadows = j.at("shadows").get<std::vector<Shadow>>();
}

inline void from_json(const json& j, TextLineAttr& x)
{
  x.level = j.at("level");
  x.firstLine = j.at("isFirst");
  x.lineType = j.at("styleType");
}

inline void from_json(const json& j, Bound2& b)
{
  auto x = j.value("x", 0.f);
  auto y = j.value("y", 0.f);
  const auto topLeft = glm::vec2{ x, y };
  auto width = j.value("width", 0.f);
  auto height = j.value("height", 0.f);
  b = Bound2{ topLeft, width, height };
}

inline void from_json(const json& j, TextStyleAttr& x)
{
  x.length = get_stack_optional<size_t>(j, "length").value_or(false);
  x.bold = get_stack_optional<bool>(j, "bold").value_or(false);
  x.italic = get_stack_optional<bool>(j, "italic").value_or(false);
  x.fontName = get_stack_optional<std::string>(j, "name").value_or("");
  x.subFamilyName = get_stack_optional<std::string>(j, "subFamilyName").value_or("");
  x.fillUseType = get_stack_optional<int>(j, "fillUseType").value_or(0);
  x.fills = j.value("fills", std::vector<Fill>());
  x.baselineShift = j.value("baselineShift", 0.0);
  x.lineThrough = j.value("linethrough", false);
  x.letterSpacing = j.value("letterSpacingValue", 0.0);
  x.lineSpace = j.value("lineSpaceValue", 0.0);
  x.underline = j.value("underline", UT_None);
  x.kerning = j.value("kerning", false);
  x.horzAlignment = j.value("horizontalAlignment", HA_Left);
  x.size = j.value("size", 14);
}

inline void from_json(const json& j, AlphaMask& x)
{
  x.id = j.value("id", "");
  x.type = j.value("alphaType", AM_ALPHA);
  x.crop = j.value("crop", true);
}
// NOLINTEND
} // namespace VGG
