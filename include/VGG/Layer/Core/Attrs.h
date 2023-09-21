#pragma once

#include "VGG/Math/Hash.hpp"
#include "VGG/Math/Math.hpp"
#include "VGG/Math/Geometry.hpp"
#include "VGG/Layer/Core/VType.h"

#include <include/effects/SkGradientShader.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/core/SkMatrix.h>
#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <algorithm>
#include <vector>
#include <stdint.h>

namespace VGG
{

struct ContextSetting
{
  EBlendMode BlendMode{ BM_Normal };
  float Opacity{ 1.0 };
  bool IsolateBlending{ false };
  EKnockoutType TransparencyKnockoutGroup{ KT_Off };
};

struct Pattern
{
  EImageFillType imageFillType;
  bool tileMirrored;
  float tileScale;
  std::string imageGUID;
  glm::mat3 transform;
};

struct Color
{
  float r{ 0. };
  float g{ 0. };
  float b{ 0. };
  float a{ 1. };

  operator SkColor() const
  {
    return SkColorSetARGB(255 * a, 255 * r, 255 * g, 255 * b);
  }

  inline static Color fromRGB(unsigned char ur, unsigned char ug, unsigned char ub)
  {
    return Color{ ur / 255.f, ug / 255.f, ub / 255.f, 1. };
  }

  inline static Color fromARGB(unsigned char ua,
                               unsigned char ur,
                               unsigned char ug,
                               unsigned char ub)
  {
    return Color{ ur / 255.f, ug / 255.f, ub / 255.f, ua / 255.f };
  }
};

template<>
inline Color lerp(const Color& a, const Color& b, double t)
{
  return Color{
    lerp(a.r, b.r, t),
    lerp(a.g, b.g, t),
    lerp(a.b, b.b, t),
    lerp(a.a, b.a, t),
  };
}

struct Gradient
{
  struct GradientStop
  {
    static constexpr double minPos = 0.0;
    static constexpr double maxPos = 1.0;
    Color color{ 1., 1., 1., 1. };
    float position{ 1.0 }; // [0,1]
    float midPoint;
  };
  static constexpr double minElipseLength = 0.01;
  EGradientType gradientType{ EGradientType::GT_Linear };

  glm::vec2 from{ 0.5, 0 };
  glm::vec2 to{ 0.5, 1 };
  float elipseLength{ 1.0 };  // (0, inf) : radial
  float rotation{ 0.0 };      // degree : angular
  float invert{ false };      //
  bool aiCoordinate{ false }; // This flag indicates if these positions are Ai exported.
  std::vector<GradientStop> stops{
    { Color::fromRGB(0xEE, 0xEE, 0xEE), 0.0 },
    { Color::fromRGB(0xD8, 0xD8, 0xD8), 1.0 },
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

  inline glm::vec2 convert(const glm::vec2& p, const Bound2& b) const
  {
    return glm::vec2{ p.x + b.topLeft.x, p.y - b.height() + b.topLeft.y };
  }

  inline std::pair<glm::vec2, glm::vec2> aiConvert(const glm::vec2& from,
                                                   const glm::vec2& to,
                                                   const Bound2& b) const
  {
    const auto angle = to.x;
    const auto length = to.y;
    const auto radians = glm::radians(angle);
    glm::vec2 dir = { std::cos(radians), std::sin(radians) };
    const auto center = glm::vec2(b.topLeft.x + b.width() / 2, b.topLeft.y - b.height() / 2);
    const auto x = from.x * b.width() + b.topLeft.x;
    float k = 1;
    if ((angle <= 45 && angle >= -45) || (angle >= 135 || angle <= -135))
    {
      const float k = dir.y / dir.x;
    }
    else
    {
      const float k = dir.x / dir.y;
    }
    glm::vec2 f = { x, k * (x - center.x) + center.y };
    glm::vec2 t = f + length * dir * b.distance();
    return { f, t };
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

  inline sk_sp<SkShader> getLinearShader(const Bound2& bound) const
  {
    auto indices = getSortedIndices();

    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.f, 1.f, 0.0001f);

    auto f = bound.map(bound.size() * from);
    auto t = bound.map(bound.size() * to);
    if (aiCoordinate)
    {
      auto r = aiConvert(from, to, bound);
      f = r.first;
      t = r.second;
    }
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
    return SkGradientShader::MakeLinear(pts,
                                        colors.data(),
                                        positions.data(),
                                        indices.size(),
                                        SkTileMode::kClamp,
                                        0,
                                        &mat);
  }

  inline sk_sp<SkShader> getRadialShader(const Bound2& bound) const
  {
    // ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();
    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.0f, 1.0f, 0.0001f);

    auto f = bound.map(bound.size() * from);
    auto t = bound.map(bound.size() * to);

    // auto f = bound.size() * from;
    // auto t = bound.size() * to;
    // if (aiCoordinate)
    // {
    //   auto r = aiConvert(from, to, bound);
    //   f = r.first;
    //   t = r.second;
    // }
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
    mat.postRotate(rad2deg(getTheta()));
    mat.postTranslate(start.x, start.y);
    // mat.postScale(1, -1);
    return SkGradientShader::MakeRadial(center,
                                        r,
                                        colors.data(),
                                        positions.data(),
                                        indices.size(),
                                        SkTileMode::kClamp,
                                        0,
                                        &mat);
  }

  inline sk_sp<SkShader> getAngularShader(const Bound2& bound) const
  {
    // ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();
    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.f, 1.f, 0.0001f);
    auto f = bound.map(bound.size() * from);
    auto t = bound.map(bound.size() * to);
    // if (aiCoordinate)
    // {
    //   auto r = aiConvert(from, to, bound);
    //   f = r.first;
    //   t = r.second;
    // }
    auto center = f;

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
    for (auto i = 0; i < indices.size(); i++)
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
    const auto dir = t - f;
    const auto rotate = std::atan2(dir.y, dir.x);
    rot.setRotate(glm::degrees(rotate), center.x, center.y);
    return SkGradientShader::MakeSweep(center.x,
                                       center.y,
                                       colors.data(),
                                       positions.data(),
                                       sz,
                                       0,
                                       &rot);
  }
};

struct Border
{
  std::optional<Color> color;
  ContextSetting context_settings;
  double dashed_offset;
  std::vector<float> dashed_pattern;
  EPathFillType fill_type; // TODO:
  double flat;
  std::optional<Gradient> gradient;
  bool isEnabled;
  ELineCap lineCapStyle;
  ELineJoin lineJoinStyle;
  double miterLimit;
  std::optional<Pattern> pattern;
  EPathPosition position;
  int64_t style;
  double thickness;
};

struct Shadow
{
  float blur;
  Color color;
  ContextSetting context_settings;
  bool inner;
  bool is_enabled;
  float offset_x;
  float offset_y;
  float spread;
};

struct Blur
{
  EBlurType blurType;
  float radius;
  float motionAngle;
  glm::vec2 center;
  float saturation;
  bool isEnabled;
};

struct Fill
{
  bool isEnabled{ true };
  Color color;
  EPathFillType fillType{};
  ContextSetting contextSettings{};
  std::optional<Gradient> gradient{ std::nullopt };
  std::optional<Pattern> pattern{ std::nullopt };
};

struct Style
{
  std::vector<Blur> blurs;
  std::vector<Border> borders;
  std::vector<Fill> fills;
  std::vector<Shadow> shadows;
  std::optional<std::array<float, 4>> frameRadius;
};

struct TextLineAttr
{
  bool firstLine{ false };
  int level{ 0 };
  int lineType{ TLT_Plain };
};

struct TextAttr
{
  std::string fontName;
  std::string subFamilyName;
  Color color{ 0, 0, 0, 1 };
  float letterSpacing{ 0.0 };
  float baselineShift{ 0.0 };
  size_t length{ 0 };
  uint8_t size{ 14 };
  bool bold{ false };
  bool italic{ false };
  bool lineThrough{ false };
  bool kerning{ false };
  ETextUnderline underline{ UT_None };
  ETextHorizontalAlignment horzAlignment{ HA_Left };
  ELetterTransform letterTransform{ ELT_Nothing };

  bool operator==(const TextAttr& other) const
  {
    size_t h = 0;
    hash_combine(h,
                 fontName,
                 subFamilyName,
                 letterSpacing,
                 length,
                 size,
                 bold,
                 italic,
                 lineThrough,
                 underline,
                 horzAlignment);
    return h;
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

struct Contour : public std::vector<PointAttr>
{
  bool closed = true;
  EBoolOp blop;
};

using ContourPtr = std::shared_ptr<Contour>;

} // namespace VGG
