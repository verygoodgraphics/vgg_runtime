#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include "Attrs.h"
using nlohmann::json;

namespace VGG
{

template<typename T>
inline std::optional<T> get_stack_optional(const json& j, const char* property)
{
  auto it = j.find(property);
  if (it != j.end() && !it->is_null())
  {
    return j.at(property).get<std::optional<T>>();
  }
  return std::optional<T>();
}

template<typename T>
inline std::optional<T> get_stack_optional(const json& j, std::string property)
{
  return get_stack_optional<T>(j, property.data());
}

inline void from_json(const json& j, VGGGradient& x)
{
  // x.instance = get_stack_optional<std::variant<std::vector<InstanceElement>,
  //                                              bool,
  //                                              double,
  //                                              int64_t,
  //                                              std::map<std::string, nlohmann::json>,
  //                                              std::string>>(j, "instance");
}

inline void from_json(const json& j, VGGColor& x)
{
  x.a = j.at("alpha").get<double>();
  x.b = j.at("blue").get<double>();
  x.g = j.at("green").get<double>();
  x.r = j.at("red").get<double>();
}

inline void from_json(const json& j, ContextSetting& x)
{
  x.BlendMode = (EBlendMode)j.at("blendMode").get<int64_t>();
  x.IsolateBlending = j.at("isolateBlending").get<bool>();
  x.Opacity = j.at("opacity").get<double>();
  // x.Opacity[0] = v[0];
  // x.Opacity[1] = v[1];
  x.TransparencyKnockoutGroup = j.at("transparencyKnockoutGroup").get<int64_t>();
}

inline void from_json(const json& j, Border& x)
{
  x.color = get_stack_optional<VGGColor>(j, "color");
  x.context_settings = j.at("contextSettings").get<ContextSetting>();
  x.dashed_offset = j.at("dashedOffset").get<double>();
  x.dashed_pattern = j.at("dashedPattern").get<std::vector<double>>();
  x.fill_type = j.at("fillType").get<int64_t>();
  x.flat = j.at("flat").get<double>();
  x.gradient = get_stack_optional<VGGGradient>(j, "gradient");
  x.is_enabled = j.at("isEnabled").get<bool>();
  x.line_cap_style = j.at("lineCapStyle").get<int64_t>();
  x.line_join_style = j.at("lineJoinStyle").get<int64_t>();
  x.miter_limit = j.at("miterLimit").get<double>();
  // x.pattern = get_stack_optional<Pattern>(j, "pattern");
  x.position = j.at("position").get<int64_t>();
  x.style = j.at("style").get<int64_t>();
  x.thickness = j.at("thickness").get<double>();
}

inline void from_json(const json& j, Shadow& x)
{
  x.blur = j.at("blur").get<double>();
  x.color = j.at("color").get<VGGColor>();
  x.context_settings = j.at("contextSettings").get<ContextSetting>();
  x.inner = j.at("inner").get<bool>();
  x.is_enabled = j.at("isEnabled").get<bool>();
  x.offset_x = j.at("offsetX").get<double>();
  x.offset_y = j.at("offsetY").get<double>();
  x.spread = j.at("spread").get<double>();
}

inline void from_json(const json& j, Blur& x)
{
  x.radius = j.at("radius").get<double>();
  x.motionAngle = j.at("motionAngle").get<double>();
  const auto v = j.at("center").get<std::vector<double>>();
  x.center.x = v[0];
  x.center.y = v[1];
  x.saturation = j.at("saturation").get<double>();
}

inline void from_json(const json& j, Fill& x)
{
  x.color = j.at("color").get<VGGColor>();
  x.fillType = (EFillType)j.at("fillType").get<int>();
  x.isEnabled = j.at("isEnabled").get<bool>();
  x.contextSettings = j.at("contextSettings").get<ContextSetting>();
}

inline void from_json(const json& j, Style& x)
{
  x.blurs = j.at("blurs").get<std::vector<Blur>>();
  x.borders = j.at("borders").get<std::vector<Border>>();
  x.fills = j.at("fills").get<std::vector<Fill>>();
  x.shadows = j.at("shadows").get<std::vector<Shadow>>();
}

inline void from_json(const json& j, TextStyleStub& x)
{
  x.length = get_stack_optional<size_t>(j, "length").value_or(false);
  x.bold = get_stack_optional<bool>(j, "bold").value_or(false);
  x.italic = get_stack_optional<bool>(j, "italic").value_or(false);
  x.fontName = get_stack_optional<std::string>(j, "name").value_or("");
  x.fillColor = get_stack_optional<VGGColor>(j, "fillColor");
}
} // namespace VGG
