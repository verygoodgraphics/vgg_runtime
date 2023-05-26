#pragma once
#include "Components/Styles.hpp"
#include "VGGType.h"
#include <ostream>
#include <skia/include/effects/SkGradientShader.h>
#include <skia/include/effects/SkDashPathEffect.h>
#include "src/core/SkRecordPattern.h"
#include <algorithm>
#include "Utils/Math.hpp"
#include <optional>
#include <vector>
#include <stdint.h>
#include <glm/glm.hpp>

namespace VGG
{
struct ContextSetting
{
  EBlendMode BlendMode;
  float Opacity;
  bool IsolateBlending;
  EKnockoutType TransparencyKnockoutGroup;
};

struct Pattern
{
  EImageFillType imageFillType;
  bool tileMirrored;
  float tileScale;
  std::string imageGUID;
  glm::mat3 transform;
};

// Type with 'VGG' prefix is temporary. It's used to distinguish
// the existsing version. Because new version doesn't need the definition
// with NLOHMANN_DEFINE_TYPE_INTRUSIVE.
// 'VGG' prefix will be removed once the old rendering code is replaced completely.
struct VGGColor
{
  float r{ 0. };
  float g{ 0. };
  float b{ 0. };
  float a{ 1. };

  operator SkColor() const
  {
    return SkColorSetARGB(255 * a, 255 * r, 255 * g, 255 * b);
  }

  inline static VGGColor fromRGB(unsigned char ur, unsigned char ug, unsigned char ub)
  {
    return VGGColor{ ur / 255.f, ug / 255.f, ub / 255.f, 1. };
  }

  inline static VGGColor fromARGB(unsigned char ua,
                                  unsigned char ur,
                                  unsigned char ug,
                                  unsigned char ub)
  {
    return VGGColor{ ur / 255.f, ug / 255.f, ub / 255.f, ua / 255.f };
  }
};

template<>
inline VGGColor lerp(const VGGColor& a, const VGGColor& b, double t)
{
  return VGGColor{
    lerp(a.r, b.r, t),
    lerp(a.g, b.g, t),
    lerp(a.b, b.b, t),
    lerp(a.a, b.a, t),
  };
}

struct VGGGradient
{
  struct GradientStop
  {
    static constexpr double minPos = 0.0;
    static constexpr double maxPos = 1.0;
    VGGColor color{ 1., 1., 1., 1. };
    float position{ 1.0 }; // [0,1]
    float midPoint;
  };
  static constexpr double minElipseLength = 0.01;
  EGradientType gradientType{ EGradientType::GT_Linear };

  glm::vec2 from{ 0.5, 0 };
  glm::vec2 to{ 0.5, 1 };
  float elipseLength{ 1.0 }; // (0, inf) : radial
  float rotation{ 0.0 };     // degree : angular
  float invert{ false };     //
  std::vector<GradientStop> stops{
    { VGGColor::fromRGB(0xEE, 0xEE, 0xEE), 0.0 },
    { VGGColor::fromRGB(0xD8, 0xD8, 0xD8), 1.0 },
  };

  inline double getTheta() const
  {
    const auto r = glm::distance(from, to);
    if (r > 0)
    {
      const auto theta = (to.x > from.x ? 1 : -1) * std::acos((to.y - from.y) / r);
      return theta;
    }
    return 0;
  }

  inline std::vector<size_t> getSortedIndices() const
  {
    std::vector<size_t> indices;
    for (size_t i = 0; i < stops.size(); i++)
    {
      indices.push_back(i);
    }
    std::sort(indices.begin(),
              indices.end(),
              [&](size_t i, size_t j) { return stops[i].position < stops[j].position; });
    return indices;
  }

  inline sk_sp<SkShader> getLinearShader(const glm::vec2& size) const
  {
    auto indices = getSortedIndices();

    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.f, 1.f, 0.0001f);

    auto f = size * from;
    auto t = size * to;
    auto start = glm::mix(f, t, minPosition);
    auto end = glm::mix(f, t, maxPosition);

    SkPoint pts[2] = {
      { (SkScalar)start.x, (SkScalar)start.y },
      { (SkScalar)end.x, (SkScalar)end.y },
    };
    std::vector<SkColor> colors;
    std::vector<SkScalar> positions;
    for (size_t i = 0; i < indices.size(); i++)
    {
      colors.push_back(stops[indices[i]].color);
      auto p = stops[indices[i]].position;
      positions.push_back((p - minPosition) / (maxPosition - minPosition));
    }
    SkMatrix mat;
    mat.postScale(1, -1);
    return SkGradientShader::MakeLinear(pts,
                                        colors.data(),
                                        positions.data(),
                                        indices.size(),
                                        SkTileMode::kClamp,
                                        0,
                                        &mat);
  }

  inline sk_sp<SkShader> getRadialShader(const glm::vec2& size) const
  {
    // ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();
    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.0f, 1.0f, 0.0001f);

    auto f = size * from;
    auto t = size * to;
    auto start = glm::mix(f, t, minPosition);
    auto end = glm::mix(f, t, maxPosition);

    SkPoint center{ (SkScalar)start.x, (SkScalar)start.y };
    SkScalar r = glm::distance(end, start);
    std::vector<SkColor> colors;
    std::vector<SkScalar> positions;
    for (size_t i = 0; i < indices.size(); i++)
    {
      colors.push_back(stops[indices[i]].color);
      auto p = stops[indices[i]].position;
      positions.push_back((p - minPosition) / (maxPosition - minPosition));
    }
    SkMatrix mat;
    mat.postTranslate(-start.x, -start.y);
    mat.postScale(elipseLength, 1.0);
    mat.postRotate(-rad2deg(getTheta()));
    mat.postTranslate(start.x, start.y);
    mat.postScale(1, -1);
    return SkGradientShader::MakeRadial(center,
                                        r,
                                        colors.data(),
                                        positions.data(),
                                        indices.size(),
                                        SkTileMode::kClamp,
                                        0,
                                        &mat);
  }

  inline sk_sp<SkShader> getAngularShader(const glm::vec2& size) const
  {
    // ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();
    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.f, 1.f, 0.0001f);
    auto f = size * from;
    auto t = size * to;
    auto center = (f + t) / 2.0f;

    std::vector<SkColor> colors;
    std::vector<SkScalar> positions;
    auto minPosColor = stops[indices[0]].color;
    auto maxPosColor = stops[indices[indices.size() - 1]].color;
    size_t sz = indices.size();
    if (minPosition > 0)
    {
      auto c = lerp(minPosColor, maxPosColor, (float)minPosition / (minPosition + 1 - maxPosition));
      colors.push_back(c);
      positions.push_back(0);
      sz += 1;
    }
    for (size_t i = 0; i < indices.size(); i++)
    {
      colors.push_back(stops[indices[i]].color);
      positions.push_back(stops[indices[i]].position);
    }
    if (maxPosition < 1)
    {
      auto c = lerp(minPosColor, maxPosColor, (float)minPosition / (minPosition + 1 - maxPosition));
      colors.push_back(c);
      positions.push_back(1);
      sz += 1;
    }

    SkMatrix rot;
    const auto dir = to - from;
    auto r = std::atan2(dir.y, dir.x);
    rot.setRotate(r, f.x, f.y);
    rot.postScale(1, -1);
    return SkGradientShader::MakeSweep(f.x, f.y, colors.data(), positions.data(), sz, 0, &rot);
  }
};

struct Border
{
  std::optional<VGGColor> color;
  ContextSetting context_settings;
  double dashed_offset;
  std::vector<float> dashed_pattern;
  EPathFillType fill_type; // TODO:
  double flat;
  std::optional<VGGGradient> gradient;
  bool is_enabled;
  ELineCap line_cap_style;
  ELineJoin line_join_style;
  double miter_limit;
  std::optional<Pattern> pattern;
  EPathPosition position;
  int64_t style;
  double thickness;
};

struct Shadow
{
  float blur;
  VGGColor color;
  ContextSetting context_settings;
  bool inner;
  bool is_enabled;
  float offset_x;
  float offset_y;
  float spread;
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
  EPathFillType fillType;
  ContextSetting contextSettings;
  std::optional<VGGGradient> gradient;
  std::optional<Pattern> pattern;
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

struct PointAttr
{
  glm::vec2 point;
  float radius = 0.0;
  std::optional<glm::vec2> from;
  std::optional<glm::vec2> to;
  std::optional<int> cornerStyle;

  PointAttr(glm::vec2 point,
            float radius,
            std::optional<glm::vec2> from,
            std::optional<glm::vec2> to,
            std::optional<int> cornerStyle)
    : point(point)
    , radius(radius)
    , from(from)
    , to(to)
    , cornerStyle(cornerStyle)
  {
  }

  EPointMode mode() const
  {
    if (from.has_value() || to.has_value())
      return EPointMode::PM_Disconnected;
    return EPointMode::PM_Straight;
  }
};

} // namespace VGG
