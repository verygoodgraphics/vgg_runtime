#pragma once
#include "Components/Styles.hpp"
#include "VGGType.h"
#include "src/core/SkRecordPattern.h"
#include <algorithm>
#include <optional>
#include <vector>
#include <stdint.h>
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

struct TextStyleStub
{
  std::optional<VGGColor> fillColor;
  std::optional<VGGColor> boarderColor;
  size_t length{ 0 };
  float lineSpace{ 1.0 };
  float paraSpacing{ 0.0 };
  float letterSpacing{ 0.0 };
  std::string fontName;
  std::optional<int> boarderSize;
  ETextVerticalAlignment vertAlignment{ VA_Top };
  ETextHorizontalAlignment horzAlignment{ HA_Left };
  ETextUnderline underline{ UT_None };
  uint8_t size{ 14 };
  bool lineThrough{ false };
  bool bold{ false };
  bool italic{ false };
  SkFont getFont() const
  {
    if (auto tf = TypefaceManager::getTypeface(fontName))
    {
      SkFont font(tf, size);
      return font;
    }
    else if (auto tf = TypefaceManager::getTypefaceByPSName(fontName))
    {
      SkFont font(tf, size);
      return font;
    }
    auto sf = SkFont(nullptr, size);
    auto tf = sf.getTypefaceOrDefault();
    ASSERT(tf);
    if (tf->countGlyphs() < 1)
    {
      sf.setTypeface(FiraSans::getSkTypeface());
    }
    return sf;
  }
  SkPaint getPaint() const
  {
    SkPaint pen;
    pen.setAntiAlias(true);
    pen.setStyle(SkPaint::kFill_Style);
    pen.setColor(fillColor.value());
    return pen;
  }
};

} // namespace VGG
