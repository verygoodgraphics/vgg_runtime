/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __STYLES_HPP__
#define __STYLES_HPP__

#include <nlohmann/json.hpp>
#include <optional>
#include <vector>
#include <algorithm>
#include <regex>
#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathEffect.h"

#include "Components/Frame.hpp"
#include "Presets/Fonts/FiraSans.hpp"
#include "Utils/Math.hpp"
#include "Utils/TypefaceManager.hpp"
#include "Utils/Types.hpp"
#include "Utils/TextureManager.hpp"
#include "Utils/DPI.hpp"

/** Sketch Incompatibilies
 *
 * REMOVES
 * *) FillStyle::FillType::NOISE implementation because it's deprecated in newer version of Sketch
 * and only effective in old verions of Sketch
 * *) EndPointMarker because the same effect can be implemented by composition and has much more
 * flexibility
 *
 * CHANGES
 * *) FillStyle::FillType::PATTERN to IMAGE
 * *) text styles
 * *) bitmap layer to path with image fill
 *
 * IMPROVES
 * *) mirrored tile and tile directions
 * *) TODO Gradient::GradientType::TWOPTSCONICAL
 * *) TODO custom shader fill for both fill and border
 * *) TODO image fill for border
 */

namespace VGG
{

struct Color
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Color, r, g, b, a);

  double r{ 0. };
  double g{ 0. };
  double b{ 0. };
  double a{ 1. };

  inline static Color fromRGB(unsigned char ur, unsigned char ug, unsigned char ub)
  {
    return Color{ ur / 255., ug / 255., ub / 255., 1. };
  }

  inline static Color fromARGB(unsigned char ua,
                               unsigned char ur,
                               unsigned char ug,
                               unsigned char ub)
  {
    return Color{ ur / 255., ug / 255., ub / 255., ua / 255. };
  }

  operator SkColor() const
  {
    return SkColorSetARGB(255 * a, 255 * r, 255 * g, 255 * b);
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

namespace Colors
{
inline static constexpr Color Black{};
inline static constexpr Color White{ 1., 1., 1. };
inline static constexpr Color Red{ 1., 0., 0. };
inline static constexpr Color Green{ 0., 1., 0. };
inline static constexpr Color Blue{ 0., 0., 1. };
inline static constexpr Color Cyan{ 0., 1., 1. };
inline static constexpr Color Magenta{ 1., 0., 1. };
inline static constexpr Color Yellow{ 1., 1., 0. };
}; // namespace Colors

struct GraphicsContextSettings
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GraphicsContextSettings, opacity, blendMode);

  static constexpr double minOpacity = 0.0;
  static constexpr double maxOpacity = 1.0;
  double opacity{ 1.0 }; // [0,1]
  SkBlendMode blendMode{ SkBlendMode::kSrcOver };
};

struct Gradient
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Gradient, gradientType, from, to, elipseLength, rotation, stops);

  enum class GradientType
  {
    LINEAR = 0,
    RADIAL = 1,
    ANGULAR = 2,
  };
  struct GradientStop
  {
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GradientStop, color, position);

    static constexpr double minPos = 0.0;
    static constexpr double maxPos = 1.0;
    Color color{ 1., 1., 1., 1. };
    double position{ 1.0 }; // [0,1]
  };
  static constexpr double minElipseLength = 0.01;

  GradientType gradientType{ GradientType::LINEAR };
  Vec2 from{ 0.5, 0 };
  Vec2 to{ 0.5, 1 };
  double elipseLength{ 1.0 }; // (0, inf)
  double rotation{ 0.0 };     // degree
  std::vector<GradientStop> stops{
    { Color::fromRGB(0xEE, 0xEE, 0xEE), 0.0 },
    { Color::fromRGB(0xD8, 0xD8, 0xD8), 1.0 },
  };

  inline double getTheta() const
  {
    const auto r = (to - from).len();
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

  inline sk_sp<SkShader> getLinearShader(const Frame& frame) const
  {
    ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();

    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.0, 1.0, 0.0001);

    auto f = frame.posFromNormalized(from.x, from.y);
    auto t = frame.posFromNormalized(to.x, to.y);
    auto start = lerp(f, t, minPosition);
    auto end = lerp(f, t, maxPosition);

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
    return SkGradientShader::MakeLinear(pts,
                                        colors.data(),
                                        positions.data(),
                                        indices.size(),
                                        SkTileMode::kClamp);
  }

  inline sk_sp<SkShader> getRadialShader(const Frame& frame) const
  {
    ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();

    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.0, 1.0, 0.0001);

    auto f = frame.posFromNormalized(from.x, from.y);
    auto t = frame.posFromNormalized(to.x, to.y);
    auto start = lerp(f, t, minPosition);
    auto end = lerp(f, t, maxPosition);

    SkPoint center{ (SkScalar)start.x, (SkScalar)start.y };
    SkScalar r = (end - start).len();
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

    return SkGradientShader::MakeRadial(center,
                                        r,
                                        colors.data(),
                                        positions.data(),
                                        indices.size(),
                                        SkTileMode::kClamp,
                                        0,
                                        &mat);
  }

  inline sk_sp<SkShader> getAngularShader(const Frame& frame) const
  {
    ASSERT(stops.size() > 1);
    auto indices = getSortedIndices();

    auto minPosition = stops[indices[0]].position;
    auto maxPosition = stops[indices[indices.size() - 1]].position;
    clampPairByLimits(minPosition, maxPosition, 0.0, 1.0, 0.0001);

    auto f = frame.posFromNormalized(from.x, from.y);
    auto t = frame.posFromNormalized(to.x, to.y);
    auto center = (f + t) / 2.0;

    std::vector<SkColor> colors;
    std::vector<SkScalar> positions;
    auto minPosColor = stops[indices[0]].color;
    auto maxPosColor = stops[indices[indices.size() - 1]].color;
    size_t sz = indices.size();
    if (minPosition > 0)
    {
      auto c = lerp(minPosColor, maxPosColor, minPosition / (minPosition + 1 - maxPosition));
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
      auto c = lerp(minPosColor, maxPosColor, minPosition / (minPosition + 1 - maxPosition));
      colors.push_back(c);
      positions.push_back(1);
      sz += 1;
    }

    SkMatrix rot;
    rot.setRotate(rotation, center.x, center.y);
    return SkGradientShader::MakeSweep(center.x,
                                       center.y,
                                       colors.data(),
                                       positions.data(),
                                       sz,
                                       0,
                                       &rot);
  }
};

struct FillStyle
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(FillStyle,
                                 fillType,
                                 color,
                                 gradient,
                                 imageName,
                                 imageFillType,
                                 imageTileScale,
                                 imageTileMirrored,
                                 imageTileDirection,
                                 noiseIndex,
                                 noiseIntensity,
                                 contextSettings);

  enum class FillType
  {
    FLAT = 0,
    GRADIENT = 1,
    IMAGE = 4,
    NOISE = 5,
  };
  enum class ImageFillType
  {
    TILE = 0,
    FILL = 1,
    STRETCH = 2,
    FIT = 3,
  };
  enum class NoiseIndex
  {
    ORIGINAL = 0,
    BLACK = 1,
    WHITE = 2,
    COLOR = 3,
  };
  enum class ImageTileDirection
  {
    BOTH = 0,
    HORZ = 1,
    VERT = 2,
  };

  static constexpr double minImageTileScale = 0.1;
  static constexpr double maxImageTileScale = 2.0;
  FillType fillType{ FillType::FLAT };
  Color color{ Color::fromRGB(0xD8, 0xD8, 0xD8) };
  Gradient gradient;
  std::optional<std::string> imageName;
  ImageFillType imageFillType{ ImageFillType::FILL };
  double imageTileScale{ 1.0 }; // [0.1,2]
  bool imageTileMirrored{ false };
  ImageTileDirection imageTileDirection{ ImageTileDirection::BOTH };
  NoiseIndex noiseIndex{ NoiseIndex::ORIGINAL };
  double noiseIntensity{ 0.0 }; // [0,1] TODO
  GraphicsContextSettings contextSettings;

  inline sk_sp<SkShader> getImageShader(const Frame& frame) const
  {
    if (!(imageName.has_value()))
    {
      return nullptr;
    }
    if (auto img = TextureManager::getSkiaImage(imageName.value()))
    {
      SkTileMode modeX = SkTileMode::kDecal;
      SkTileMode modeY = SkTileMode::kDecal;
      SkMatrix mat = SkMatrix::I();
      SkImageInfo mi = img->imageInfo();

      if (imageFillType == ImageFillType::FILL)
      {
        double sx = frame.w / mi.width();
        double sy = frame.h / mi.height();
        double s = std::max(sx, sy);
        mat.postScale(s, s);
        if (sx > sy)
        {
          // scaled image's width == frame's width
          mat.postTranslate(0, (frame.h - sx * mi.height()) / 2);
        }
        else
        {
          // scaled image's height == frame's height
          mat.postTranslate((frame.w - sy * mi.width()) / 2, 0);
        }
      }
      else if (imageFillType == ImageFillType::FIT)
      {
        double sx = frame.w / mi.width();
        double sy = frame.h / mi.height();
        double s = std::min(sx, sy);
        mat.postScale(s, s);
        if (sx < sy)
        {
          // scaled image's width == frame's width
          mat.postTranslate(0, (frame.h - sx * mi.height()) / 2);
        }
        else
        {
          // scaled image's height == frame's height
          mat.postTranslate((frame.w - sy * mi.width()) / 2, 0);
        }
      }
      else if (imageFillType == ImageFillType::STRETCH)
      {
        double sx = frame.w / mi.width();
        double sy = frame.h / mi.height();
        mat.postScale(sx, sy);
      }
      else if (imageFillType == ImageFillType::TILE)
      {
        mat.postScale(imageTileScale, imageTileScale);
        if (imageTileDirection == ImageTileDirection::HORZ ||
            imageTileDirection == ImageTileDirection::BOTH)
        {
          modeX = imageTileMirrored ? SkTileMode::kMirror : SkTileMode::kRepeat;
        }
        if (imageTileDirection == ImageTileDirection::VERT ||
            imageTileDirection == ImageTileDirection::BOTH)
        {
          modeY = imageTileMirrored ? SkTileMode::kMirror : SkTileMode::kRepeat;
        }
      }
      SkSamplingOptions opt;
      return img->makeShader(modeX, modeY, opt, &mat);
    }
    return nullptr;
  }
};

struct BorderStyle
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(BorderStyle,
                                 fillType,
                                 color,
                                 thickness,
                                 position,
                                 gradient,
                                 contextSettings);

  static constexpr double minThickness = 0.5;
  enum class FillType
  {
    FLAT = 0,
    GRADIENT = 1,
  };
  enum class BorderPosition
  {
    CENTER = 0,
    INSIDE = 1,
    OUTSIDE = 2,
  };

  FillType fillType{ FillType::FLAT };
  Color color{ Color::fromRGB(0x97, 0x97, 0x97) };
  double thickness{ 1.0 };
  BorderPosition position{ BorderPosition::CENTER };
  Gradient gradient;
  GraphicsContextSettings contextSettings;
};

struct BorderOptions
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(BorderOptions,
                                 dashPattern,
                                 lineCapStyle,
                                 lineJoinStyle,
                                 miterLimit);

  static constexpr double minMiterLimit = 0.0;

  enum class LineCap
  {
    BUTT = 0,
    ROUND = 1,
    SQUARE = 2,
  };
  enum class LineJoin
  {
    MITER = 0,
    ROUND = 1,
    BEVEL = 2,
  };

  std::vector<double> dashPattern;
  LineCap lineCapStyle{ LineCap::BUTT };
  LineJoin lineJoinStyle{ LineJoin::MITER };
  double miterLimit{ 10. };

  inline void getDashPatternAsChar64(char buf[64])
  {
    if (dashPattern.size() < 1)
    {
      snprintf(buf, 64, "");
      return;
    }
    std::string s = scalarfmt(dashPattern[0], 2);
    for (size_t i = 1; i < dashPattern.size(); i++)
    {
      auto d = scalarfmt(dashPattern[i], 2);
      s += ";";
      s += d;
    }
    snprintf(buf, 64, "%s", s.c_str());
  }

  inline void setDashPatternFromChar64(char buf[64])
  {
    std::regex n("(\\d+(\\.\\d*)?|\\.\\d+)");
    std::string s(buf);
    auto i = std::sregex_iterator(s.begin(), s.end(), n);
    auto end = std::sregex_iterator();
    if (std::distance(i, end) < 1)
    {
      return;
    }
    dashPattern.clear();
    while (i != end)
    {
      std::string s = (*i).str();
      double v = std::stod(s);
      dashPattern.push_back(std::round(100. * v) / 100.);
      i++;
    }
  }

  inline sk_sp<SkPathEffect> getDashEffect() const
  {
    std::vector<SkScalar> dashes;
    for (auto d : dashPattern)
    {
      dashes.push_back((SkScalar)d);
    }
    return SkDashPathEffect::Make(dashes.data(), dashes.size(), 0);
  }

  inline SkPaint::Join getSkiaLineJoin() const
  {
    switch (lineJoinStyle)
    {
      case LineJoin::MITER:
      {
        return SkPaint::kMiter_Join;
      }
      case LineJoin::ROUND:
      {
        return SkPaint::kRound_Join;
      }
      case LineJoin::BEVEL:
      {
        return SkPaint::kBevel_Join;
      }
    }
    ASSERT(false);
  }

  inline SkPaint::Cap getSkiaLineCap() const
  {
    switch (lineCapStyle)
    {
      case LineCap::BUTT:
      {
        return SkPaint::kButt_Cap;
      }
      case LineCap::ROUND:
      {
        return SkPaint::kRound_Cap;
      }
      case LineCap::SQUARE:
      {
        return SkPaint::kSquare_Cap;
      }
    }
    ASSERT(false);
  }
};

struct ShadowStyle
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShadowStyle,
                                 color,
                                 blurRadius,
                                 offset,
                                 spreadRatio,
                                 contextSettings);

  static constexpr double minBlurRadius = 0.0;
  static constexpr double minSpreadRatio = 0.1;
  static constexpr double maxSpreadRatio = 10.;

  Color color{ Color::fromARGB(0x7f, 0, 0, 0) };
  double blurRadius{ 4.0 };
  Vec2 offset{ 0, 2 };
  double spreadRatio{
    1.0
  }; // the range should be reciprocal-invariant to limit both shadows and inner shadows
  GraphicsContextSettings contextSettings;
};

struct BlurStyle
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(BlurStyle, blurType, radius, center, motionAngle, saturation);

  static constexpr double minRadius = 0.0;
  static constexpr double maxRadius = 50.0;
  enum class BlurType
  {
    GAUSSIAN = 0,
    MOTION = 1,
    ZOOM = 2,
    BACKGROUND = 3,
  };

  BlurType blurType{ BlurType::GAUSSIAN }; // TODO implementation other than gaussian
  double radius{ 10.0 };                   // [0,50] all blur
  Vec2 center{ 0.5, 0.5 };                 // zoom blur
  double motionAngle{ 0.0 };               // [-180,180] motion blur
  double saturation{ 1.0 };                // [0,2] background blur
};

template<typename T>
struct StyleSwitch
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(StyleSwitch<T>, isEnabled, style);

  bool isEnabled{ false };
  T style;
};

struct PathStyle
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PathStyle,
                                 windingRule,
                                 fills,
                                 borders,
                                 borderOptions,
                                 shadows,
                                 innerShadows,
                                 blur,
                                 contextSettings);

  enum class WindingRule
  {
    NONZERO = 0,
    EVENODD = 1,
  };

  WindingRule windingRule{ WindingRule::EVENODD };
  std::vector<StyleSwitch<FillStyle>> fills;
  std::vector<StyleSwitch<BorderStyle>> borders;
  BorderOptions borderOptions;
  std::vector<StyleSwitch<ShadowStyle>> shadows;
  std::vector<StyleSwitch<ShadowStyle>> innerShadows;
  StyleSwitch<BlurStyle> blur;
  GraphicsContextSettings contextSettings;

  inline bool isFilled() const
  {
    for (auto& fill : fills)
    {
      if (fill.isEnabled)
      {
        return true;
      }
    }
    return false;
  }

  inline bool isStroked() const
  {
    for (auto& border : borders)
    {
      if (border.isEnabled)
      {
        return true;
      }
    }
    return false;
  }
};

struct TextStyle
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextStyle,
                                 typeface,
                                 size,
                                 color,
                                 horzAlignment,
                                 vertAlignment,
                                 lineHeightRatio,
                                 letterSpacing,
                                 paraSpacing,
                                 isStrikeThrough,
                                 isUnderline);

  enum class HorzAlignment
  {
    HA_LEFT = 0,
    HA_CENTER = 2,
    HA_RIGHT = 1,
  };
  enum class VertAlignment
  {
    VA_TOP = 0,
    VA_CENTER = 1,
    VA_BOTTOM = 2,
  };

  static constexpr double minSpacing = 0.0;
  static constexpr double minLineHeightRatio = 0.1;
  static constexpr double maxLineHeightRatio = 20.;
  std::string typeface{ "WenQuanYi Micro Hei, Regular" };
  double size{ 14.0 };
  Color color{ Colors::Black };
  HorzAlignment horzAlignment{ HorzAlignment::HA_LEFT };
  VertAlignment vertAlignment{ VertAlignment::VA_TOP };
  double lineHeightRatio{ 1.0 };
  double letterSpacing{ 0.0 };
  double paraSpacing{ 0.0 };
  bool isStrikeThrough{ false };
  bool isUnderline{ false };

  inline SkFont getFont() const
  {
    if (auto tf = TypefaceManager::getTypeface(typeface))
    {
      SkFont font(tf, size);
      return font;
    }
    else if (auto tf = TypefaceManager::getTypefaceByPSName(typeface))
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

  inline SkPaint getPaint() const
  {
    SkPaint pen;
    pen.setAntiAlias(true);
    pen.setStyle(SkPaint::kFill_Style);
    pen.setColor(color);
    return pen;
  }
};

}; // namespace VGG

#endif // __STYLES_HPP__
