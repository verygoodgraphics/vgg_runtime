/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
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
#ifndef __SKETCH_IMPORTER_HPP__
#define __SKETCH_IMPORTER_HPP__

#include <nlohmann/json.hpp>
#include <skia/include/core/SkTypes.h>
#include <skia/include/core/SkBlendMode.h>

#include "Components/Frame.hpp"
#include "Components/Path.hpp"
#include "Components/Text.hpp"
#include "Components/Relation.hpp"
#include "Components/Styles.hpp"
#include "Entity/Entity.hpp"
#include "Presets/Paths/RectPath.hpp"
#include "Presets/Styles/DefaultPathStyle.hpp"
#include "Utils/EntityContainer.hpp"
#include "Utils/TextCodecs.hpp"
#include "Utils/FileUtils.hpp"

//#define DEBUG2(...) DEBUG(__VA_ARGS__)
#define DEBUG2(...)

namespace VGG
{

namespace SketchImporter
{

using json = nlohmann::json;

#define DEFINE_JSON_GETTER(NAME, DEFAULT_VALUE)                                                    \
  inline auto get##NAME(const json& js, const char* key, json&& val = DEFAULT_VALUE)               \
  {                                                                                                \
    return js.value(key, val);                                                                     \
  }
DEFINE_JSON_GETTER(ObjOrNull, json::value_t::null)
DEFINE_JSON_GETTER(Obj, json::value_t::object)
DEFINE_JSON_GETTER(Str, json::value_t::string)
DEFINE_JSON_GETTER(Array, json::value_t::array)
DEFINE_JSON_GETTER(Scalar, json::value_t::number_float)
DEFINE_JSON_GETTER(Integer, json::value_t::number_integer)
#undef DEFINE_JSON_GETTER

inline std::optional<Vec2> fromSketchVec2(const std::string& s)
{
  DEBUG2("fromSketchVec2");
  if (!(s.rfind('{', 0) == 0 && s.rfind('}') + 1 == s.size()))
  {
    FAIL("Not a sketch vec2.");
    return std::nullopt;
  }
  if (s.size() < 3)
  {
    return Vec2{};
  }
  auto sub = s.substr(1, s.size() - 2);
  for (auto n = sub.find(","); n != std::string::npos; n = sub.find(","))
  {
    sub.replace(n, 1, " ");
  }
  size_t ss;
  double x = 0;
  double y = 0;
  try
  {
    x = std::stod(sub, &ss);
    y = std::stod(sub.substr(ss));
  }
  catch (const std::invalid_argument& err)
  {
  }
  return Vec2{ x, y };
}

template<typename T>
inline std::vector<T> fromSketchVector(const json& sv)
{
  DEBUG2("fromSketchVector");
  std::vector<T> v;
  if (!(sv.is_array()))
  {
    FAIL("Not a sketch vector.");
    return v;
  }

  for (auto i : sv)
  {
    v.push_back(i);
  }
  return v;
}

#define DEF_VECTOR_OF(T, F)                                                                        \
  template<>                                                                                       \
  inline std::vector<T> fromSketchVector(const json& sv)                                           \
  {                                                                                                \
    DEBUG2("fromSketchVector: " STR(T));                                                           \
    std::vector<T> v;                                                                              \
    if (!(sv.is_array()))                                                                          \
    {                                                                                              \
      FAIL("Not a sketch vector of " STR(T));                                                      \
      return v;                                                                                    \
    }                                                                                              \
    for (auto i : sv)                                                                              \
    {                                                                                              \
      if (auto opt = F(i))                                                                         \
      {                                                                                            \
        v.push_back(opt.value());                                                                  \
      }                                                                                            \
    }                                                                                              \
    return v;                                                                                      \
  }

inline std::optional<Frame> fromSketchRect(const json& rect)
{
  DEBUG2("fromSketchRect");
  if (rect.is_null())
  {
    return Frame{};
  }
  if (getStr(rect, "_class") != "rect")
  {
    FAIL("Not a sketch rect.");
    return std::nullopt;
  }
  return Frame{
    getScalar(rect, "x"),
    getScalar(rect, "y"),
    getScalar(rect, "width"),
    getScalar(rect, "height"),
  };
}

inline std::optional<Color> fromSketchColor(const json& c)
{
  DEBUG2("fromSketchColor");
  if (c.is_null())
  {
    return Colors::Black;
  }
  if (getStr(c, "_class") != "color")
  {
    FAIL("Not a sketch color.");
    return std::nullopt;
  }
  return Color{
    getScalar(c, "red"),
    getScalar(c, "green"),
    getScalar(c, "blue"),
    getScalar(c, "alpha", 1.),
  };
}

inline std::optional<CurvePoint> fromSketchCurvePoint(const json& curvePoint)
{
  DEBUG2("fromSketchCurvePoint");
  if (getStr(curvePoint, "_class") != "curvePoint")
  {
    FAIL("Not a sketch curve point.");
    return std::nullopt;
  }
  return CurvePoint{
    .mode = CurvePoint::PointMode(getInteger(curvePoint, "curveMode")),
    .radius = getScalar(curvePoint, "cornerRadius"),
    .point = fromSketchVec2(getStr(curvePoint, "point")).value(),
    .from = getInteger(curvePoint, "hasCurveFrom") ? fromSketchVec2(getStr(curvePoint, "curveFrom"))
                                                   : std::nullopt,
    .to = getInteger(curvePoint, "hasCurveTo") ? fromSketchVec2(getStr(curvePoint, "curveTo"))
                                               : std::nullopt,
  };
}
DEF_VECTOR_OF(CurvePoint, fromSketchCurvePoint);

inline Path::Specialization fromSketchPathSpecialization(const std::string& special)
{
  using SP = Path::Specialization;
  if (special == "rectangle")
    return SP::RECTANGLE;
  if (special == "oval")
    return SP::OVAL;
  if (special == "triangle")
    return SP::TRIANGLE;
  if (special == "star")
    return SP::STAR;
  if (special == "polygon")
    return SP::POLYGON;
  return SP::NORMAL;
}

inline std::optional<Path> fromSketchPath(const json& path,
                                          const std::string& special = "shapePath")
{
  DEBUG2("fromSketchPath: %s", special.c_str());
  if (getStr(path, "_class") != special)
  {
    FAIL("Not a sketch path.");
    return std::nullopt;
  }
  return Path{
    .isClosed = getInteger(path, "isClosed"),
    .points = fromSketchVector<CurvePoint>(getArray(path, "points")),
    .special = fromSketchPathSpecialization(special),
  };
}

inline std::optional<BorderOptions> fromSketchBorderOptions(const json& bo, int sm, int em)
{
  DEBUG2("fromSketchBorderOptions");
  if (bo.is_null())
  {
    return BorderOptions{};
  }
  if (getStr(bo, "_class") != "borderOptions")
  {
    FAIL("Not a sketch border options.");
    return std::nullopt;
  }
  auto dp = fromSketchVector<double>(getArray(bo, "dashPattern"));
  if (dp.size() == 1)
  {
    // Sketch only supports [dash, gap] as dashPattern,
    // and assume gap == dash if gap is not given.
    dp.push_back(dp[0]);
  }
  return BorderOptions{
    .dashPattern = dp,
    .lineCapStyle = BorderOptions::LineCap(getInteger(bo, "lineCapStyle")),
    .lineJoinStyle = BorderOptions::LineJoin(getInteger(bo, "lineJoinStyle")),
  };
}

inline std::optional<StyleSwitch<BlurStyle>> fromSketchBlur(const json& blur)
{
  DEBUG2("fromSketchBlur");
  if (blur.is_null())
  {
    return StyleSwitch<BlurStyle>{};
  }
  if (getStr(blur, "_class") != "blur")
  {
    FAIL("Not a sketch blur.");
    return std::nullopt;
  }
  return StyleSwitch<BlurStyle>{
    .isEnabled = getInteger(blur, "isEnabled"),
    .style =
      BlurStyle{
        .blurType = BlurStyle::BlurType(getInteger(blur, "type")),
        .radius = getScalar(blur, "radius", 10.),
        .center = fromSketchVec2(getStr(blur, "center")).value(),
        .motionAngle = getScalar(blur, "motionAngle"),
        .saturation = getScalar(blur, "saturation", 1.),
      },
  };
}

inline std::optional<GraphicsContextSettings> fromSketchGraphicsContextSettings(const json& cs)
{
  DEBUG2("fromSketchGraphicsContextSettings");
  if (cs.is_null())
  {
    return GraphicsContextSettings{};
  }
  if (getStr(cs, "_class") != "graphicsContextSettings")
  {
    FAIL("Not a sketch graphics context settings.");
    return std::nullopt;
  }
  static SkBlendMode modes[] = {
    SkBlendMode::kSrcOver,    // normal
    SkBlendMode::kDarken,     // darken
    SkBlendMode::kMultiply,   // multiply
    SkBlendMode::kColorBurn,  // color burn TODO
    SkBlendMode::kLighten,    // lighten
    SkBlendMode::kScreen,     // screen
    SkBlendMode::kColorDodge, // color dodge TODO
    SkBlendMode::kOverlay,    // overlay
    SkBlendMode::kSoftLight,  // soft light
    SkBlendMode::kHardLight,  // hard light
    SkBlendMode::kDifference, // difference
    SkBlendMode::kExclusion,  // exclusion
    SkBlendMode::kHue,        // hue
    SkBlendMode::kSaturation, // saturation
    SkBlendMode::kColor,      // color
    SkBlendMode::kLuminosity, // luminosity
    SkBlendMode::kModulate,   // plus darker TODO
    SkBlendMode::kPlus,       // plus lighter
  };
  int bmIdx = getInteger(cs, "blendMode");
  ASSERT(bmIdx < SK_ARRAY_COUNT(modes));
  return GraphicsContextSettings{
    .opacity = getScalar(cs, "opacity", 1.),
    .blendMode = modes[bmIdx],
  };
}

inline std::optional<Gradient::GradientStop> fromSketchGradientStop(const json& stop)
{
  DEBUG2("fromSketchGradientStop");
  if (getStr(stop, "_class") != "gradientStop")
  {
    FAIL("Not a sketch gradient stop.");
    return std::nullopt;
  }
  return Gradient::GradientStop{
    .color = fromSketchColor(getObjOrNull(stop, "color")).value(),
    .position = getScalar(stop, "position", 1.),
  };
}
DEF_VECTOR_OF(Gradient::GradientStop, fromSketchGradientStop);

inline std::optional<Gradient> fromSketchGradient(const json& g)
{
  DEBUG2("fromSketchGradient");
  if (g.is_null())
  {
    return Gradient{};
  }
  if (getStr(g, "_class") != "gradient")
  {
    FAIL("Not a sketch gradient");
    return std::nullopt;
  }
  double el = getScalar(g, "elipseLength", 1.);
  return Gradient{
    .gradientType = Gradient::GradientType(getInteger(g, "gradientType")),
    .from = fromSketchVec2(getStr(g, "from")).value(),
    .to = fromSketchVec2(getStr(g, "to")).value(),
    .elipseLength = (el < FZERO) ? 1. : el,
    .stops = fromSketchVector<Gradient::GradientStop>(getArray(g, "stops")),
  };
}

inline std::optional<StyleSwitch<FillStyle>> fromSketchFill(const json& fill)
{
  DEBUG2("fromSketchFill");
  if (getStr(fill, "_class") != "fill")
  {
    FAIL("Not a sketch fill.");
    return std::nullopt;
  }
  auto imageName = FileUtils::getFileName(getStr(getObj(fill, "image"), "_ref"));
  // TODO completely remove noise fill type by converting to image fill when importing
  return StyleSwitch<FillStyle>{
    .isEnabled = getInteger(fill, "isEnabled"),
    .style =
      FillStyle{
        .fillType = FillStyle::FillType(getInteger(fill, "fillType")),
        .color = fromSketchColor(getObjOrNull(fill, "color")).value(),
        .gradient = fromSketchGradient(getObjOrNull(fill, "gradient")).value(),
        .imageName = imageName.empty() ? std::nullopt : std::make_optional(imageName),
        .imageFillType = FillStyle::ImageFillType(getInteger(fill, "patternFillType")),
        .imageTileScale = getScalar(fill, "patternTileScale", 1.),
        .noiseIndex = FillStyle::NoiseIndex(getInteger(fill, "noiseIndex")),
        .noiseIntensity = getScalar(fill, "noiseIntensity"),
        .contextSettings =
          fromSketchGraphicsContextSettings(getObjOrNull(fill, "contextSettings")).value(),
      },
  };
}
DEF_VECTOR_OF(StyleSwitch<FillStyle>, fromSketchFill);

inline std::optional<StyleSwitch<BorderStyle>> fromSketchBorder(const json& border)
{
  DEBUG2("fromSketchBorder");
  if (getStr(border, "_class") != "border")
  {
    FAIL("Not a sketch border.");
    return std::nullopt;
  }
  return StyleSwitch<BorderStyle>{
    .isEnabled = getInteger(border, "isEnabled"),
    .style =
      BorderStyle{
        .fillType = BorderStyle::FillType(getInteger(border, "fillType")),
        .color = fromSketchColor(getObjOrNull(border, "color")).value(),
        .thickness = getScalar(border, "thickness", 1.),
        .position = BorderStyle::BorderPosition(getInteger(border, "position")),
        .gradient = fromSketchGradient(getObjOrNull(border, "gradient")).value(),
        .contextSettings =
          fromSketchGraphicsContextSettings(getObjOrNull(border, "contextSettings")).value(),
      },
  };
}
DEF_VECTOR_OF(StyleSwitch<BorderStyle>, fromSketchBorder);

inline std::optional<StyleSwitch<ShadowStyle>> fromSketchShadow(const json& shadow)
{
  DEBUG2("fromSketchShadow");
  std::string _class = getStr(shadow, "_class");
  if (_class != "shadow" && _class != "innerShadow")
  {
    FAIL("Not a sketch shadow.");
    return std::nullopt;
  }
  return StyleSwitch<ShadowStyle>{
    .isEnabled = getInteger(shadow, "isEnabled"),
    .style =
      ShadowStyle{
        .color = fromSketchColor(getObjOrNull(shadow, "color")).value(),
        .blurRadius = getScalar(shadow, "blurRadius", 4.),
        .offset =
          Vec2{
            getScalar(shadow, "offsetX"),
            getScalar(shadow, "offsetY", 2.),
          },
        .contextSettings =
          fromSketchGraphicsContextSettings(getObjOrNull(shadow, "contextSettings")).value(),
      },
  };
}
DEF_VECTOR_OF(StyleSwitch<ShadowStyle>, fromSketchShadow);
#undef DEF_VCTOR_OF

inline std::optional<PathStyle> fromSketchPathStyle(const json& style)
{
  DEBUG2("fromSketchPathStyle");
  if (style.is_null())
  {
    return PathStyle{};
  }
  if (getStr(style, "_class") != "style")
  {
    FAIL("Not a sketch path style.");
    return std::nullopt;
  }
  return PathStyle{
    .windingRule = PathStyle::WindingRule(getInteger(style, "windingRule")),
    .fills = fromSketchVector<StyleSwitch<FillStyle>>(getArray(style, "fills")),
    .borders = fromSketchVector<StyleSwitch<BorderStyle>>(getArray(style, "borders")),
    .borderOptions = fromSketchBorderOptions(getObjOrNull(style, "borderOptions"),
                                             getInteger(style, "startMarkerType"),
                                             getInteger(style, "endMarkerType"))
                       .value(),
    .shadows = fromSketchVector<StyleSwitch<ShadowStyle>>(getArray(style, "shadows")),
    .innerShadows = fromSketchVector<StyleSwitch<ShadowStyle>>(getArray(style, "innerShadows")),
    .blur = fromSketchBlur(getObjOrNull(style, "blur")).value(),
    .contextSettings =
      fromSketchGraphicsContextSettings(getObjOrNull(style, "contextSettings")).value(),
  };
}

inline std::optional<std::wstring> fromSketchAttributedString(const json& as)
{
  DEBUG2("fromSketchAttributedString");
  if (as.is_null())
  {
    return L"";
  }
  if (getStr(as, "_class") != "attributedString")
  {
    FAIL("Not a sketch attributed string.");
    return std::nullopt;
  }
  return TextCodecs::conv.from_bytes(getStr(as, "string"));
}

inline std::optional<TextStyle> fromSketchTextStyle(const json& ts)
{
  DEBUG2("fromSketchTextStyle");
  if (ts.is_null())
  {
    return TextStyle{};
  }
  if (getStr(ts, "_class") != "textStyle")
  {
    FAIL("Not a sketch text style.");
    return std::nullopt;
  }
  auto ea = getObj(ts, "encodedAttributes");
  auto fd = getObj(ea, "MSAttributedStringFontAttribute");
  auto ps = getObj(ea, "paragraphStyle");
  auto fdAttr = getObj(fd, "attributes");
  return TextStyle{
    .typeface = getStr(fdAttr, "name", "Fira Sans, Regular"),
    .size = getScalar(fdAttr, "size", 14.),
    .color = fromSketchColor(getObjOrNull(ea, "MSAttributedStringColorAttribute")).value(),
    .horzAlignment = TextStyle::HorzAlignment(getInteger(ps, "alignment")),
    .vertAlignment = TextStyle::VertAlignment(getInteger(ts, "verticalAlignment")),
    .lineHeightRatio = 1.0,
    .letterSpacing = getScalar(ea, "kerning"),
    .paraSpacing = getScalar(ps, "paragraphSpacing"),
    .isStrikeThrough = (getInteger(ea, "strikethroughStyle") != 0),
    .isUnderline = (getInteger(ea, "underlineStyle") != 0),
  };
}

inline Frame getVGGFrame(const json& layer)
{
  auto frame = fromSketchRect(getObjOrNull(layer, "frame")).value();
  frame.rotation = getScalar(layer, "rotation");
  frame.rotation *= -1.;
  frame.flipX = getInteger(layer, "isFlippedHorizontal");
  frame.flipY = getInteger(layer, "isFlippedVertical");
  return frame;
}

inline std::optional<Entity> fromSketchLayer(const json& layer)
{
  std::string type = getStr(layer, "_class", "MISSING");
  DEBUG2("fromSketchLayer: type=%s", type.c_str());

  if ("group" == type || "shapeGroup" == type || "artboard" == type)
  {
    EntityContainer children;

    for (auto sublayer : getArray(layer, "layers"))
    {
      if (auto entOpt = fromSketchLayer(sublayer))
      {
        children.push(entOpt.value());
      }
    }

    return Entity().renderable<FramedRelation>(FramedRelation{
      .frame = getVGGFrame(layer),
      .children = children,
    });
  }
  else if ("text" == type)
  {
    auto s = getObj(layer, "style");
    return Entity().renderable<FramedText>(FramedText{
      .frame = getVGGFrame(layer),
      .text = fromSketchAttributedString(getObjOrNull(layer, "attributedString")).value(),
      .style = fromSketchTextStyle(getObjOrNull(s, "textStyle")).value(),
    });
  }
  else if ("rectangle" == type || "oval" == type || "triangle" == type || "star" == type ||
           "polygon" == type || "shapePath" == type)
  {
    return Entity().renderable<FramedPath>(FramedPath{
      .frame = getVGGFrame(layer),
      .path = fromSketchPath(layer, type).value(),
      .style = fromSketchPathStyle(getObjOrNull(layer, "style")).value(),
    });
  }
  else if ("bitmap" == type)
  {
    auto imageName = FileUtils::getFileName(getStr(getObj(layer, "image"), "_ref"));
    if (imageName.empty())
    {
      return Entity().renderable<FramedPath>(FramedPath{
        .frame = getVGGFrame(layer),
        .path = RectPath::create(),
        .style = DefaultPathStyle::create(),
      });
    }
    auto style = fromSketchPathStyle(getObjOrNull(layer, "style")).value();
    style.fills = {
      StyleSwitch<FillStyle>{
        .isEnabled = true,
        .style =
          FillStyle{
            .fillType = FillStyle::FillType::IMAGE,
            .imageName = imageName,
            .imageFillType = FillStyle::ImageFillType::STRETCH,
          },
      },
    };
    return Entity().renderable<FramedPath>(FramedPath{
      .frame = getVGGFrame(layer),
      .path = RectPath::create(),
      .style = style,
    });
  }

  WARN("Unsupported layer: %s", type.c_str());

  return std::nullopt;
}

inline double fromSketchRulerDataBase(const json& rd)
{
  DEBUG2("fromSketchRulerDataBase");
  if (rd.is_null())
  {
    return 0;
  }
  if (getStr(rd, "_class") != "rulerData")
  {
    FAIL("Not a sketch ruler data.");
    return 0;
  }
  return getScalar(rd, "base");
}

inline std::optional<EntityContainer> fromSketchPage(const json& page)
{
  DEBUG2("fromSketchPage");
  if (getStr(page, "_class") != "page")
  {
    FAIL("Not a sketch page.");
    return std::nullopt;
  }

  EntityContainer container;

  for (auto layer : getArray(page, "layers"))
  {
    if (auto entOpt = fromSketchLayer(layer))
    {
      container.push(entOpt.value());
    }
  }

  double horzBase = fromSketchRulerDataBase(getObjOrNull(page, "horizontalRulerData"));
  double vertBase = fromSketchRulerDataBase(getObjOrNull(page, "verticalRulerData"));

  container.applyOffset(Vec2{ -horzBase, -vertBase });

  return container;
}

}; // namespace SketchImporter

}; // namespace VGG

#endif // __SKETCH_IMPORTER_HPP__
