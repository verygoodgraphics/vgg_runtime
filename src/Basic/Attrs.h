#pragma once
#include "Components/Styles.hpp"
#include "VGGType.h"
#include "src/core/SkRecordPattern.h"
#include <optional>
#include <vector>
#include <stdint.h>

namespace VGG
{
struct ContextSetting
{
  EBlendMode BlendMode;
  float Opacity[2];
  bool IsolateBlending;
  int TransparencyKnockoutGroup;
};

struct Pattern
{
};

struct Border
{
  std::optional<Color> color;
  ContextSetting context_settings;
  double dashed_offset;
  std::vector<double> dashed_pattern;
  int64_t fill_type;
  double flat;
  std::optional<Gradient> gradient;
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
  Color color;
  GraphicsContextSettings context_settings;
  bool inner;
  bool is_enabled;
  double offset_x;
  double offset_y;
  double spread;
};

struct Style
{
  // std::vector<nlohmann::json> blurs;
  std::vector<Border> borders;
  // std::vector<nlohmann::json> fills;
  std::vector<Shadow> shadows;
};
} // namespace VGG
