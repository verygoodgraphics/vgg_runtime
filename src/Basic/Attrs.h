#pragma once
#include "Components/Styles.hpp"
#include "VGGType.h"
#include "src/core/SkRecordPattern.h"
#include <optional>
#include <vector>
#include <stdint.h>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

namespace nlohmann
{
} // namespace nlohmann

namespace VGG
{
struct ContextSetting
{
  EBlendMode BlendMode;
  float Opacity;
  bool IsolateBlending;
  int TransparencyKnockoutGroup;
};

struct Pattern
{
};

struct VGGColor
{
  float r;
  float g;
  float b;
  float a;

  operator SkColor() const
  {
    return SkColorSetARGB(255 * a, 255 * r, 255 * g, 255 * b);
  }
};

struct VGGGradient
{
};

struct Border
{
  std::optional<VGGColor> color;
  ContextSetting context_settings;
  double dashed_offset;
  std::vector<double> dashed_pattern;
  int64_t fill_type;
  double flat;
  std::optional<VGGGradient> gradient;
  bool is_enabled;
  int64_t line_cap_style;
  int64_t line_join_style;
  double miter_limit;
  std::optional<Pattern> pattern;
  int64_t position;
  int64_t style;
  double thickness;
};

struct Shadow
{
  double blur;
  VGGColor color;
  ContextSetting context_settings;
  bool inner;
  bool is_enabled;
  double offset_x;
  double offset_y;
  double spread;
};

struct Blur
{
  float radius;
  float motionAngle;
  glm::vec2 center;
  float saturation;
};

struct Fill
{
  bool isEnabled;
  VGGColor color;
  EFillType fillType;
  ContextSetting contextSettings;
};

struct Style
{
  std::vector<Blur> blurs;
  std::vector<Border> borders;
  std::vector<Fill> fills;
  std::vector<Shadow> shadows;
};

using nlohmann::json;

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

} // namespace VGG
