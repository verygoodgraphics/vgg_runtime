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

#include "DesignModelFwd.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <variant>

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann
{
template<typename T>
struct adl_serializer<std::shared_ptr<T>>
{
  static void to_json(json& j, const std::shared_ptr<T>& opt)
  {
    if (!opt)
      j = nullptr;
    else
      j = *opt;
  }

  static std::shared_ptr<T> from_json(const json& j)
  {
    if (j.is_null())
      return std::make_shared<T>();
    else
      return std::make_shared<T>(j.get<T>());
  }
};
template<typename T>
struct adl_serializer<std::optional<T>>
{
  static void to_json(json& j, const std::optional<T>& opt)
  {
    if (!opt)
      j = nullptr;
    else
      j = *opt;
  }

  static std::optional<T> from_json(const json& j)
  {
    if (j.is_null())
      return std::make_optional<T>();
    else
      return std::make_optional<T>(j.get<T>());
  }
};
} // namespace nlohmann
#endif

namespace VGG
{
namespace Model
{
using nlohmann::json;

struct PatternImageFill;
struct PatternImageStrech;
struct PatternImageFit;
struct PatternImageTile;
struct PatternLayerInstance;
using PatternInstanceType = std::variant<
  std::monostate,
  PatternImageFill,
  PatternImageStrech,
  PatternImageFit,
  PatternImageTile,
  PatternLayerInstance>;

#ifndef NLOHMANN_UNTYPED_VGG_Model_HELPER
#define NLOHMANN_UNTYPED_VGG_Model_HELPER
inline json get_untyped(const json& j, const char* property)
{
  if (j.find(property) != j.end())
  {
    return j.at(property).get<json>();
  }
  return json();
}

inline json get_untyped(const json& j, std::string property)
{
  return get_untyped(j, property.data());
}

template<typename T>
inline void safeGetTo(T& to, const json& j, const char* property)
{
  if (j.contains(property))
  {
    to = j.at(property).get<T>();
  }
}
#endif

#ifndef NLOHMANN_OPTIONAL_VGG_Model_HELPER
#define NLOHMANN_OPTIONAL_VGG_Model_HELPER
template<typename T>
inline std::shared_ptr<T> get_heap_optional(const json& j, const char* property)
{
  auto it = j.find(property);
  if (it != j.end() && !it->is_null())
  {
    return j.at(property).get<std::shared_ptr<T>>();
  }
  return std::shared_ptr<T>();
}

template<typename T>
inline std::shared_ptr<T> get_heap_optional(const json& j, std::string property)
{
  return get_heap_optional<T>(j, property.data());
}
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
#endif

enum class AlphaMaskByClass : int
{
  ALPHA_MASK
};

/**
 * Define the alpha mask effect for the object, using some values of the mask to control the
 * transparency of the object.
 */
struct AlphaMask
{
  /**
   * The data type whose values are used to control the transparency of the object.
   */
  int              alphaType;
  AlphaMaskByClass alphaMaskClass;
  /**
   * Whether to use the mask's boundary to crop the masked object.
   */
  bool             crop;
  /**
   * Object ID of the mask.
   */
  std::string      id;
};

enum class BackgroundColorClass : int
{
  COLOR
};

/**
 * The background color of the canvas on which the frame is positioned. Exists only on the
 * top-level frame.
 *
 * A RGBA color value.
 *
 * The color of the fill.
 *
 * The color of the border's fill.
 *
 * The color of the shadow.
 */
struct Color
{
  /**
   * The opacity of the object. `0` represents transparent, and `1` represents opaque.
   */
  float                alpha;
  float                blue;
  BackgroundColorClass colorClass;
  float                green;
  float                red;
};

enum class BoundsClass : int
{
  RECT
};

/**
 * The bounds of the object before undergoing matrix transformations.
 *
 * A rectangle.
 *
 * An enclosing rectangle for the object. This property stores information about the object
 * after the matrix transformation.
 */
struct Rect
{
  BoundsClass rectClass;
  /**
   * Whether to keep the height and width ratio constant while scaling.
   */
  bool        constrainProportions;
  double      height;
  double      width;
  /**
   * The X coordinate of the top-left corner point.
   * The X-axis increases to the right.
   */
  double      x;
  /**
   * The Y coordinate of the top-left corner point.
   * The Y-axis increases to the up.
   */
  double      y;
};

enum class ContextSettingsClass : int
{
  GRAPHICS_CONTEXT_SETTINGS
};

/**
 * The opacity and blending-related configurations of the object.
 *
 * Define the opacity, blend mode, isolation blending, and transparency knockout group of
 * the object.
 *
 * The opacity and blending-related configurations of the fill.
 *
 * The opacity and blending-related configurations of the border.
 *
 * The opacity and blending-related configurations of the shadow.
 *
 * The opacity and blending-related configurations.
 */
struct GraphicsContextSettings
{
  /**
   * The blend mode of the object.
   */
  int                  blendMode;
  ContextSettingsClass graphicsContextSettingsClass;
  /**
   * To leave objects beneath unaffected, you can isolate the blending mode to a targeted
   * layer or group.
   */
  bool                 isolateBlending;
  /**
   * The opacity of the object. `0` represents transparent, and `1` represents opaque.
   */
  double               opacity;
  /**
   * In a transparency knockout group, the elements of a group don't show through each other.
   */
  int                  transparencyKnockoutGroup;
};

enum class SubGeometryClass : int
{
  CONTOUR,
  ELLIPSE,
  FRAME,
  GROUP,
  IMAGE,
  PATH,
  POLYGON,
  RECTANGLE,
  STAR,
  SYMBOL_INSTANCE,
  SYMBOL_MASTER,
  TEXT,
  VECTOR_NETWORK
};

enum class BorderClass : int
{
  BORDER
};

enum class GradientClass : int
{
  GRADIENT
};

enum class GeometryClass : int
{
  GRADIENT_BASIC_GEOMETRY
};

/**
 * The meaning of the gradient vector is different for radial gradients than for linear
 * gradients. The vector origin is the center of the circle containing the radial gradient;
 * the vector length is the radius of the that circle. The vector angle is not used by
 * radial blends, but is preserved and used if the user changes the gradient from radial to
 * linear.
 *
 * > ***Unstable.*** It is only used to represent the gradient effect converted from AI and
 * is not the final format. It is likely to be modified in the future.
 *
 * Gradient geometry defines much of the appearance of the gradient within the path.
 */
struct GradientBasicGeometry
{
  /**
   * This argument specifies the direction of the gradient vector, in degrees. The gradient
   * ramp extends from the origin at the value of angle, which is measured counterclockwise
   * from the X axis.
   */
  double              angle;
  GeometryClass       gradientBasicGeometryClass;
  /**
   * This argument defines how the gradient will be rendered.For simple filled paths, flag
   * takes the value 1.
   */
  int                 flag;
  /**
   * This argument specifies the distance over which the gradient ramp is applied. The ramp
   * will be scaled so that its 100% value is the end of the length. This parameter may be
   * greater than 1.
   */
  double              length;
  /**
   * The six values make up a transformation matrix.
   * When a gradient is first applied to an object, these values are the identity matrix.
   * If the user transforms the object, the user transformation matrix is concatenated to the
   * gradient instance's matrix.
   */
  std::vector<double> matrix;
  /**
   * 0.5 indicates that the width is half the length.
   * 2 indicates that the width is twice the length.
   * And so on.
   */
  double              widthRatio;
  /**
   * `xOrigin` and `yOrigin` give the origin of the gradient in page coordinates. The origin
   * can be located anywhere on the artwork, and corresponds to 0 on the gradient ramp.
   */
  double              xOrigin;
  /**
   * Refer to xOriginal for details.
   */
  double              yOrigin;
};

enum class GradientInstanceClass : int
{
  GRADIENT_ANGULAR,
  GRADIENT_BASIC,
  GRADIENT_DIAMOND,
  GRADIENT_LINEAR,
  GRADIENT_RADIAL
};

enum class HilightClass : int
{
  GRADIENT_HILIGHT
};

/**
 * Only exists in radial mode.
 *
 * > ***Unstable.*** It is only used to represent the gradient effect converted from AI and
 * is not the final format. It is likely to be modified in the future.
 *
 * Radial gradients have an additional attribute called a hilight. The hilight serves at the
 * starting point for the gradient ramp as it expands outward. It is still contained within
 * the gradient vector circle.
 */
struct GradientHilight
{
  /**
   * This argument is the angle (in degrees) to the hilight point, measured counterclockwise
   * from the X axis.
   */
  double       angle;
  HilightClass gradientHilightClass;
  /**
   * This argument is the distance of the hilight from the origin, expressed as a fraction of
   * the radius—a value between 0 and 1.
   */
  double       length;
  /**
   * `xHilight` and `yHilight` specify the hilight placement, in x and y offsets from the
   * gradient vector origin.
   */
  double       xHilight;
  /**
   * Refer to `xHilight` for details.
   */
  double       yHilight;
};

struct PerpendicularMatrix
{
  int                 flag;
  std::vector<double> matrix;
};

enum class StopClass : int
{
  GRADIENT_STOP
};

/**
 * A color stop defines a position on the gradient ramp that marks the start or stop of a
 * color transition.
 */
struct GradientStop
{
  StopClass gradientStopClass;
  Color     color;
  /**
   * A position between the current color stop and the next one, where there is an equal mix
   * of the two colors.
   * `midPoint` is a percentage of the distance between the two color stops. ***Default value
   * is `0.5`.***
   * The `midPoint` is ignored for the last color stop, except in the case of angular
   * gradients.
   */
  double    midPoint;
  /**
   * A number giving the position of the color stop on the gradient ramp.
   */
  double    position;
};

/**
 * One of the gradients listed below.
 *
 * An angular gradient is a color effect that transitions radially or angularly, typically
 * originating from a central point and expanding outward in a rotational pattern.
 *
 * A linear gradient refers to the effect of transitioning between two or more different
 * colors in a straight line.
 *
 * A radial gradient is a color effect that transitions radially around a point.
 *
 * A diamond gradient is a color effect that transitions similar to the luster of a
 * diamond.
 *
 * > ***Unstable.*** This gradient format is only used to represent the gradient effect
 * converted from AI and is not the final format. It is likely to be modified in the future.
 */
struct GradientInstance
{
  GradientInstanceClass                                    gradientClass;
  std::vector<double>                                      from;
  std::vector<double>                                      to;
  std::vector<GradientStop>                                stops;
  std::optional<std::variant<std::vector<double>, double>> ellipse;
};

/**
 * The gradient of the fill.
 *
 * Define a gradient.
 *
 * The gradient of the border's fill.
 */
struct Gradient
{
  GradientClass    gradientClass;
  /**
   * One of the gradients listed below.
   */
  GradientInstance instance;
};

enum class ImageFiltersClass : int
{
  IMAGE_FILTERS
};

/**
 * Adjust the colors of the image.
 *
 * Adjust the detailed colors of an image.
 */
struct ImageFilters
{
  ImageFiltersClass     imageFiltersClass;
  std::optional<double> contrast;
  std::optional<double> exposure;
  std::optional<double> highlights;
  std::optional<double> hue;
  bool                  isEnabled;
  std::optional<double> saturation;
  std::optional<double> shadows;
  std::optional<double> temperature;
  std::optional<double> tint;
};

enum class PatternInstanceClass : int
{
  PATTERN_IMAGE_FILL,
  PATTERN_IMAGE_FIT,
  PATTERN_IMAGE_STRETCH,
  PATTERN_IMAGE_TILE,
  PATTERN_LAYER,
  PATTERN_UNKNOWN
};

/**
 * One of the patterns listed below.
 *
 * Use an image as the content of the pattern. The pattern type is fill.
 *
 * Use an image as the content of the pattern. The pattern type is stretch.
 *
 * Use an image as the content of the pattern. The pattern type is fit.
 *
 * Use an image as the content of the pattern. The pattern type is tile.
 *
 * Use a layer as the content of the pattern.
 */
struct PatternImageFill
{
  PatternInstanceClass        class_;
  std::string                 imageFileName;
  double                      rotation;
  std::optional<ImageFilters> imageFilters;
};
struct PatternImageStrech
{
  PatternInstanceClass        class_;
  std::string                 imageFileName;
  std::vector<double>         matrix;
  std::optional<ImageFilters> imageFilters;
};
struct PatternImageFit
{
  PatternInstanceClass        class_;
  std::string                 imageFileName;
  double                      rotation;
  std::optional<ImageFilters> imageFilters;
};
struct PatternImageTile
{
  PatternInstanceClass        class_;
  double                      scale;
  std::string                 imageFileName;
  double                      rotation;
  std::optional<bool>         mirror;
  std::optional<int>          mode;
  std::optional<ImageFilters> imageFilters;
};
struct PatternLayerInstance
{
  PatternInstanceClass class_;
  std::string          refLayerName;
  std::vector<double>  offset;
  std::vector<double>  scale;
  double               angle;
  bool                 reflection;
  double               r;
  double               shear;
  double               shearAxis;
  std::vector<double>  matrix;
};

enum class PatternClass : int
{
  PATTERN
};

/**
 * The pattern of the fill.
 *
 * Define a pattern.
 *
 * The pattern of the border's fill.
 */
struct Pattern
{
  PatternClass        patternClass;
  /**
   * One of the patterns listed below.
   */
  PatternInstanceType instance;
};

/**
 * Define a border style of an object.
 */
struct Border
{
  /**
   * For rectangles only. If `true`, independent border weights for all four sides.
   * Default value is `false`.
   */
  std::optional<bool>     borderWeightsIndependent;
  /**
   * This is used only when `borderWeightsIndependent` is set to true, in order to specify the
   * border weight at the bottom of the rectangle. Default value is `0`.
   */
  std::optional<double>   bottomWeight;
  BorderClass             borderClass;
  /**
   * The color of the border's fill.
   */
  std::optional<Color>    color;
  /**
   * The opacity and blending-related configurations of the border.
   */
  GraphicsContextSettings contextSettings;
  /**
   * Border dash initial offset. (applies to `dashed` borders)
   */
  double                  dashedOffset;
  /**
   * A list of values that describe the lengths of dashes (filled regions) and gaps (empty
   * regions) in a `dashed` border by repeating themselves.
   */
  std::vector<double>     dashedPattern;
  /**
   * The content types of the border's fill.
   */
  int                     fillType;
  /**
   * The flatness parameter specifies the accuracy or smoothness with which curves are
   * rendered as a sequence of flat line segments.
   * Default value is `0`.
   */
  double                  flat;
  /**
   * The gradient of the border's fill.
   */
  std::optional<Gradient> gradient;
  bool                    isEnabled;
  /**
   * This is used only when `borderWeightsIndependent` is set to true, in order to specify the
   * border weight at the left of the rectangle. Default value is `0`.
   */
  std::optional<double>   leftWeight;
  /**
   * The shape styles at the end of border lines.
   */
  int                     lineCapStyle;
  /**
   * The shape style at the corner of two border lines.
   */
  int                     lineJoinStyle;
  /**
   * When the angle between two adjacent border lines is less than this value, a `miter` join
   * is used instead.
   */
  double                  miterLimit;
  /**
   * The pattern of the border's fill.
   */
  std::optional<Pattern>  pattern;
  /**
   * The position types of the border relative to the boundary.
   */
  int                     position;
  /**
   * This is used only when `borderWeightsIndependent` is set to true, in order to specify the
   * border weight at the right of the rectangle. Default value is `0`.
   */
  std::optional<double>   rightWeight;
  /**
   * The type of the border style. The dashed style is further specified in `dashedOffset` and
   * `dashedPattern`.
   */
  int                     style;
  /**
   * The thickness of the border.
   * This attribute only works if `borderWeightsIndependent` is set to `false`.
   */
  double                  thickness;
  /**
   * This is used only when `borderWeightsIndependent` is set to true, in order to specify the
   * border weight at the top of the rectangle. Default value is `0`.
   */
  std::optional<double>   topWeight;
};

enum class FillClass : int
{
  FILL
};

/**
 * Define a fill style of an object.
 */
struct Fill
{
  FillClass               fillClass;
  /**
   * The color of the fill.
   */
  std::optional<Color>    color;
  /**
   * The opacity and blending-related configurations of the fill.
   */
  GraphicsContextSettings contextSettings;
  /**
   * The content type of the fill.
   */
  int                     fillType;
  /**
   * The gradient of the fill.
   */
  std::optional<Gradient> gradient;
  bool                    isEnabled;
  /**
   * The pattern of the fill.
   */
  std::optional<Pattern>  pattern;
};

enum class FontVariationClass : int
{
  FONT_VARIATION
};

/**
 * One key-value pair of font variation.
 */
struct FontVariation
{
  FontVariationClass fontVariationClass;
  /**
   * The official name of the font variation.
   * For example:
   * - `wght` means weight
   * - `wdth` means width
   * - `slnt` means slant
   */
  std::string        name;
  /**
   * The value of the font variation.
   */
  double             value;
};

enum class FontAttrClass : int
{
  FONT_ATTR
};

enum class TextParagraphClass : int
{
  TEXT_PARAGRAPH
};

/**
 * The properties of the text paragraph, which are consistent across the same text
 * paragraph.
 *
 * The properties of a text paragraph.
 */
struct TextParagraph
{
  TextParagraphClass textParagraphClass;
  /**
   * Additional spacing between paragraphs, in addition to line spacing.
   * Default value is `0`.
   */
  double             paragraphSpacing;
};

/**
 * The default font attributes of the text. If some font attributes are missing in
 * `fontAttr`, take the font attributes here.
 * The value of `length` in this field is meaningless.
 *
 * The font attributes of a text fragment.
 *
 * Font attributes only affect for text objects.
 */
struct TextFontAttributes
{
  /**
   * Vertical offset of the text baseline.
   * Default value is `0`.
   */
  std::optional<double>                     baselineShift;
  /**
   * A list of the character's border styles.
   * Priority: The `borders` of the text fragment > the `borders` in `defaultFontAttr` of the
   * text > the `borders` in the text object's `style`.
   */
  std::optional<std::vector<Border>>        borders;
  FontAttrClass                             textFontAttributesClass;
  /**
   * A list of the character's fill effects. The priority of `fills` is listed in
   * `fillUseType`.
   */
  std::optional<std::vector<Fill>>          fills;
  /**
   * The priority of `fills`.
   */
  std::optional<int>                        fillUseType;
  /**
   * The type of small caps.
   * **Note**:
   * `textCase` and `fontVariantCaps` are mutually exclusive. If either item is nonzero, you
   * can ignore the value of the other, as they will not be nonzero at the same time.
   */
  std::optional<int>                        fontVariantCaps;
  /**
   * The position of the text characters.
   */
  std::optional<int>                        fontVariantPosition;
  /**
   * A list of font variations.
   */
  std::optional<std::vector<FontVariation>> fontVariations;
  /**
   * Scale the text characters horizontally.
   * Default value is `1`.
   * Value examples:
   * 0.5:  50%
   * 1:    not scale
   * 1.75: 175%
   * 2:    200%
   */
  std::optional<double>                     horizontalScale;
  /**
   * Hyperlink for jump.
   */
  std::optional<std::string>                hyperlink;
  /**
   * The number of characters(UTF-8) that these attributes apply to.
   * If this property is missing, it means these attributes apply to all remaining characters
   * in the text.
   * **Note**:
   * If a UTF-8 character is `4` bytes, its length counts as `2`.
   * If a UTF-8 character is `1 ~ 3` bytes, its length counts as `1`.
   */
  std::optional<int>                        length;
  /**
   * The unit of `letterSpacingValue` value.
   */
  std::optional<int>                        letterSpacingUnit;
  /**
   * Text character spacing value (can be negative).
   * Must be used together with `letterSpacingUnit`.
   * Default value is `0`.
   */
  std::optional<double>                     letterSpacingValue;
  /**
   * The unit of `lineSpacingValue` value.
   */
  std::optional<int>                        lineSpacingUnit;
  /**
   * The spacing value of the text lines.
   * Must be used together with `lineSpacingUnit`.
   * Default value is `0`.
   */
  std::optional<double>                     lineSpacingValue;
  /**
   * Whether the text has a line through.
   * Default value is `false`.
   */
  std::optional<bool>                       linethrough;
  /**
   * Common name of the font.
   * The font name may contain subfamily, in which case the `subFamilyName` will not appear.
   * If this property does not exist or is empty, the application can fall back to using its
   * default font name.
   */
  std::optional<std::string>                name;
  /**
   * The PostScript name of the font.
   * The value may be empty.
   */
  std::optional<std::string>                postScript;
  /**
   * Rotation angle of the text characters.
   * Default value is `0`.
   */
  std::optional<double>                     rotate;
  /**
   * Font size.
   * Default value is `1`.
   */
  std::optional<double>                     size;
  /**
   * Subfamily name or font type.
   * The value may be empty.
   */
  std::optional<std::string>                subFamilyName;
  /**
   * The type of text case.
   * **Note**:
   * `textCase` and `fontVariantCaps` are mutually exclusive. If either item is nonzero, you
   * can ignore the value of the other, as they will not be nonzero at the same time.
   */
  std::optional<int>                        textCase;
  /**
   * The properties of the text paragraph, which are consistent across the same text paragraph.
   */
  std::optional<TextParagraph>              textParagraph;
  /**
   * The underline type of the text.
   */
  std::optional<int>                        underline;
  /**
   * Scale the text characters vertically.
   * Default value is `1`.
   * Refer to `horizontalScale` for value examples.
   */
  std::optional<double>                     verticalScale;
};

enum class OverrideValueClass : int
{
  OVERRIDE_VALUE
};

/**
 * Define an individual symbol override.
 */
struct OverrideValue
{
  OverrideValueClass       overrideValueClass;
  /**
   * If the value of this property is `true`, the current override applies to the layout.
   * Default value is `false`.
   * **Note**: Layout information is defined in `layout.json`, not `design.json`.
   */
  std::optional<bool>      effectOnLayout;
  /**
   * A list of IDs or keys that refer to objects, describing an overridden chain. The value of
   * the last object will be overridden. The other objects on the chain are instances.
   * **For example**:
   * Suppose this array is `[D456, 6516, FB09]`.
   * Suppose the current instance (of ID `XXXX`) points to a symbol-master (of ID `C1CF`),
   * which contains a symbol-instance (of ID `D456`).
   * And this instance (of ID `D456`) points to another symbol-master (of ID `0739`), which
   * contains another symbol-instance (of ID `6516`).
   * Recursively, this instance (of ID `6516`) points to yet another symbol-master (of ID
   * `59B5`), which contains the final object (of ID `FB09`).
   * So the current instance (of ID `XXXX`) will override the value of the final object (of ID
   * `FB09`) according to `overrideName` and `overrideValue`.
   * **Note**: Each item in the array is associated with the object's `overrideKey` first. If
   * `overrideKey` does not exist, the object's ID is associated.
   */
  std::vector<std::string> objectId;
  /**
   * Starting with the top-level property of the object, use `.` as a separator.
   * If the property is an array, it contains the index of the array, starting at `0`, and `*`
   * is valid for all items.
   * For example:
   * `style`: override whole style.
   * `style.fills`: override the `style.fills`.
   * `style.fills.0`: override the first item of `style.fills`.
   * `style.fills.0.color`: override the color of the first `style.fills`.
   * `style.fills.*.color`: override the color of all items in `style.fills`.
   */
  std::string              overrideName;
  /**
   * The value of the overridden attribute, whose type depends on the concrete property. There
   * are two special cases:
   * When `overrideName` ends with `style` and `overrideValue` conforms to
   * `referenced_style_<id>`, then it uses the corresponding value in references (top-level
   * attribute).
   * When the value is `null`, it indicates deletion.
   */
  nlohmann::json           overrideValue;
};

enum class PointClass : int
{
  POINT_ATTR
};

/**
 * One point on the contour of a shape.
 */
struct PointAttr
{
  PointClass          pointAttrClass;
  std::vector<double> point;

  std::optional<int>                 cornerStyle;
  std::optional<std::vector<double>> curveFrom;
  std::optional<std::vector<double>> curveTo;
  std::optional<int>                 markType;
  std::optional<double>              radius;
};

enum class RegionClass : int
{
  REGION
};

/**
 * A region in a vector network.
 */
struct Region
{
  RegionClass                              regionClass;
  /**
   * A list of the fill styles of the region. If it's missing, the `style.fills` of the
   * top-level object is uesd.
   */
  std::optional<std::vector<Fill>>         fills;
  /**
   * A list of loops of the region.
   * Note: A loop is a part of the region, which is not necessarily closed.
   */
  std::vector<std::vector<nlohmann::json>> loops;
  /**
   * The rule determining whether an area is inside or outside a region.
   */
  int                                      windingRule;
};

enum class SegmentClass : int
{
  SEGMENT
};

/**
 * A segment in a vector network.
 */
struct Segment
{
  SegmentClass                       segmentClass;
  /**
   * The control point of the starting point of the Bezier curve.
   */
  std::optional<std::vector<double>> curveFrom;
  /**
   * The control point of the ending point of the Bezier curve.
   */
  std::optional<std::vector<double>> curveTo;
  /**
   * The ending point of the segment. The number is the index of the `vertices` list in
   * `Vector NetWork`, starting from `0`.
   */
  int                                end;
  /**
   * The starting point of the segment. The number is the index of the `vertices` list in
   * `Vector NetWork`, starting from `0`.
   */
  int                                start;
};

enum class BlurClass : int
{
  BLUR
};

/**
 * Define a blur style of an object.
 */
struct Blur
{
  /**
   * The center of the blur applies only to the `zoom` blur. The coordinate point is in
   * normalized coordinates.
   */
  std::vector<double>   center;
  BlurClass             blurClass;
  bool                  isEnabled;
  /**
   * The angle of the blur direction applies only to the `motion` blur.
   */
  std::optional<double> motionAngle;
  /**
   * The range of the area where the blur takes effect.
   */
  std::optional<double> radius;
  /**
   * The saturation level of the blur applies only to the `background` blur.
   */
  double                saturation;
  /**
   * The type of blur.
   */
  int                   type;
};

enum class ShadowClass : int
{
  SHADOW
};

/**
 * Define a shadow style of an object.
 */
struct Shadow
{
  /**
   * The degree of shadow blur.
   */
  double                  blur;
  ShadowClass             shadowClass;
  /**
   * The color of the shadow.
   */
  Color                   color;
  /**
   * The opacity and blending-related configurations of the shadow.
   */
  GraphicsContextSettings contextSettings;
  /**
   * If `true`, it's an inner shadow.
   */
  bool                    inner;
  bool                    isEnabled;
  /**
   * The horizontal offset of the shadow in relation to the object. Direction to the right.
   */
  double                  offsetX;
  /**
   * The vertical upward offset of the shadow in relation to the object.
   */
  double                  offsetY;
  /**
   * When the object has a transparent fill and a non-inner shadow, whether the shadow is
   * visible through the transparent area.
   * Default value is `false`.
   */
  std::optional<bool>     showBehindTransparentAreas;
  /**
   * The distance that the shadow spreads from the object, the farther it is, the larger the
   * shadow.
   */
  double                  spread;
};

enum class StyleClass : int
{
  STYLE
};

/**
 * The borders, fills, and other styles of the object.
 *
 * Define an object style.
 *
 * The style attributes.
 */
struct Style
{
  /**
   * A list of the blur styles of the object.
   */
  std::vector<Blur>   blurs;
  /**
   * A list of the border styles of the object.
   */
  std::vector<Border> borders;
  StyleClass          styleClass;
  /**
   * A list of the fill styles of the object.
   */
  std::vector<Fill>   fills;
  /**
   * A list of the shadow styles of the object.
   */
  std::vector<Shadow> shadows;
};

enum class TextLineTypeClass : int
{
  TEXT_LINE_TYPE
};

/**
 * The properties of a text line within a text list.
 */
struct TextLineType
{
  TextLineTypeClass textLineTypeClass;
  /**
   * Whether the text line is the first line in the text list of the same hierarchy level.
   * Only has meaning for an ordered list.
   */
  bool              isFirst;
  /**
   * The hierarchy level of the text line, starting with `0`(default value).
   */
  int               level;
  int               styleType;
};

enum class TextOnPathClass : int
{
  TEXT_ON_PATH
};

/**
 * The text is arranged along the path.
 *
 * Text on path, allowing text to be arranged along a specified path.
 */
struct TextOnPath
{
  TextOnPathClass textOnPathClass;
};

enum class VariableAssignmentClass : int
{
  VARIABLE_ASSIGN
};

/**
 * Assign a new value to the variable.
 */
struct VariableAssign
{
  VariableAssignmentClass variableAssignClass;
  /**
   * The ID of the reassigned variable defined in `VariableDefine`.
   */
  std::string             id;
  /**
   * The new value of the variable.
   */
  nlohmann::json          value;
};

enum class VariableDefClass : int
{
  VARIABLE_DEF
};

/**
 * Define a variable.
 */
struct VariableDefine
{
  VariableDefClass variableDefineClass;
  /**
   * The ID of the variable is not globally unique but unique under the object tree.
   */
  std::string      id;
  /**
   * The value of the variable, whose type is determined by `varType`.
   * In the master, the object refers to a variable, and you don't need to care about the
   * value of the variable; you just use its own value.
   * In an instance, if the variable is not reassigned, the value of the corresponding
   * variable in the master is used.
   * In an instance, reassigning a variable affects all objects in that instance that
   * reference the variable.
   * In an instance, the direct override of a property takes precedence over the value of the
   * corresponding variable.
   */
  nlohmann::json   value;
  /**
   * The value type of the variable.
   */
  int              varType;
};

enum class VariableRefClass : int
{
  VARIABLE_REF
};

/**
 * Referencing a variable.
 */
struct VariableRefer
{
  VariableRefClass variableReferClass;
  /**
   * The ID of the referenced variable which defined in `VariableDefine`.
   */
  std::string      id;
  /**
   * Describes how a referenced variable acts on a specific property of an object.
   * When the `varType` of the variable is `2`, the field is fixed to `textData`.
   * When the `varType` is anything else, the field points to a property of the object (check
   * `overrideName` in `OverrideValue`).
   */
  std::string      objectField;
};

enum class VertexClass : int
{
  VERTEX
};

/**
 * A vertex in a vector network.
 */
struct Vertex
{
  VertexClass           vertexClass;
  /**
   * The shape type of the vertex. This property only applies to endpoints.
   */
  std::optional<int>    markType;
  /**
   * The coordinates of the point before the matrix transformation.
   */
  std::vector<double>   point;
  /**
   * Corner radius at the vertex.
   */
  std::optional<double> radius;
};

enum class SubshapeClass : int
{
  SUBSHAPE
};

enum class ShapeClass : int
{
  SHAPE
};

enum class ObjectClass : int
{
  FRAME,
  GROUP,
  IMAGE,
  PATH,
  SYMBOL_INSTANCE,
  SYMBOL_MASTER,
  TEXT
};

using ReferenceType = std::variant<std::monostate, ReferencedStyle, SymbolMaster>;

/**
 * Describes the detailed shape of the subshape through a contour with a list of points, a
 * specified geometry, or another object.
 * When the `frame`, `symbol-instance`, and `symbol-master` are in `subGeometry`, only the
 * `childObjects` will be used, and the bounding box will be ignored.
 * Any mask nested within this object will be invalidated.
 *
 * The contour of a shape.
 *
 * A vector network improves on the path model by allowing lines and curves between any two
 * points, instead of requiring that they all join up to form a single chain.
 *
 * An elliptical or sector shape. The object's bounds determine the size of the ellipse.
 *
 * A polygon shape.
 *
 * A rectangular shape, determined by the object's bounds.
 *
 * A star shape.
 *
 * Details of a text.
 *
 * Details of an image.
 *
 * Path represents a vector geometry which is formed by individual subpaths combined
 * together via boolean operations.
 *
 * A group combines a list of objects, but it is not considered an object itself.
 * In contrast to a frame, a frame is an object that can be rendered and a container that
 * holds objects.
 * For example:
 * The group's fills affect all of its children.
 * The frame's fills affect itself.
 *
 * A symbol master is a reusable object that contains a group of objects.
 *
 * Symbol instance object is an instance of a symbol master which can be overriden.
 *
 * A frame is an object that can be rendered and a container that holds objects.
 * In contrast to a group, a group is not an object but rather a collection of objects.
 * For example:
 * The frame's fills affect itself.
 * The group's fills affect all of its children.
 */
struct Contour
{
  SubGeometryClass       class_;
  bool                   closed;
  std::vector<PointAttr> points;
};
struct VectorNetwork
{
  SubGeometryClass     class_;
  std::vector<Vertex>  vertices;
  std::vector<Segment> segments;
  std::vector<Region>  regions;
};
struct Ellipse
{
  SubGeometryClass class_;
  double           startingAngle;
  double           endingAngle;
  double           innerRadius;
};
struct Polygon
{
  SubGeometryClass      class_;
  int                   pointCount;
  std::optional<double> radius;
};
struct Rectangle
{
  SubGeometryClass                   class_;
  std::optional<double>              cornerRadius;
  std::optional<std::vector<double>> radius;
};
struct Star
{
  SubGeometryClass      class_;
  double                ratio;
  int                   pointCount;
  std::optional<double> radius;
};

/**
 * One subshape in a shape.
 */
struct Subshape
{
  int                              booleanOperation;
  SubshapeClass                    subshapeClass;
  std::shared_ptr<SubGeometryType> subGeometry;

  Subshape() = default;
  Subshape(const Subshape& other);
  Subshape& operator=(const Subshape& other);
};

/**
 * The shape of the path, describing the details of the path.
 *
 * The shape of a path. Shape represents a vector geometry which is formed by individual
 * subshapes combined together via boolean operations.
 */
struct Shape
{
  ShapeClass            shapeClass;
  /**
   * The radius of the shape, when the `subshapes` have boolean operations.
   * Affects the corners of all `subshapes` under the shape, as well as the corners of the
   * overlapping region of the `subshapes`.
   * The `radius` of the subshapes override this radius.
   */
  std::optional<double> radius;
  /**
   * A list of subshapes in the shape.
   */
  std::vector<Subshape> subshapes;
  /**
   * The rule determining whether an area is inside or outside a path.
   * Note: For `vectorNetwork`, this property should be ignored.
   */
  int                   windingRule;
};

struct Object
{
  virtual ~Object() = default;

  ObjectClass class_;

  std::vector<AlphaMask>   alphaMaskBy;
  Rect                     bounds;
  GraphicsContextSettings  contextSettings;
  std::string              id;
  bool                     isLocked;
  int                      maskType;
  std::vector<double>      matrix;
  std::vector<std::string> outlineMaskBy;
  int                      overflow;
  Style                    style;
  int                      styleEffectMaskArea;
  bool                     visible;

  std::optional<double>                      cornerSmoothing;
  std::optional<int>                         horizontalConstraint;
  std::optional<bool>                        keepShapeWhenResize;
  std::optional<int>                         maskShowType;
  std::optional<std::string>                 name;
  std::optional<std::string>                 overrideKey;
  std::optional<int>                         resizesContent;
  std::optional<int>                         styleEffectBoolean;
  std::optional<Rect>                        transformedBounds;
  std::optional<std::vector<VariableDefine>> variableDefs;
  std::optional<std::vector<VariableRefer>>  variableRefs;
  std::optional<int>                         verticalConstraint;
};

struct Container : public Object
{
  std::vector<ContainerChildType> childObjects;
};

struct Path : public Object
{
  std::shared_ptr<Shape> shape;

  Path() = default;
  Path(const Path& other)
    : Object{ other }
  {
    if (other.shape)
    {
      shape = std::make_shared<Shape>(*other.shape);
    }
  }

  Path& operator=(const Path& other)
  {
    static_cast<Object&>(*this) = other;
    if (other.shape)
    {
      shape = std::make_shared<Shape>(*other.shape);
    }
    return *this;
  }
};

/**
 * A frame is an object that can be rendered and a container that holds objects.
 * In contrast to a group, a group is not an object but rather a collection of objects.
 * For example:
 * The frame's fills affect itself.
 * The group's fills affect all of its children.
 */
struct Frame : public Container
{
  std::optional<Color>               backgroundColor;
  std::optional<std::vector<double>> radius;
};

struct Group : public Container
{
  std::optional<bool> groupNestMaskType;
  std::optional<bool> isVectorNetwork;
};

struct Image : public Object
{
  std::string imageFileName;

  std::optional<bool>         fillReplacesImage;
  std::optional<ImageFilters> imageFilters;
};

struct SymbolInstance : public Object
{
  std::string                masterId;
  std::vector<OverrideValue> overrideValues;

  std::optional<std::vector<double>>         radius;
  std::optional<std::vector<VariableAssign>> variableAssignments;
};

struct SymbolMaster : public Container
{
  std::optional<std::vector<double>> radius;
};

struct Text : public Object
{
  std::string                     content;
  std::vector<TextFontAttributes> fontAttr;
  int                             frameMode;
  std::vector<int>                horizontalAlignment;
  int                             verticalAlignment;

  std::optional<std::vector<double>>       anchorPoint;
  std::optional<TextFontAttributes>        defaultFontAttr;
  std::optional<std::vector<TextLineType>> textLineType;
  std::optional<TextOnPath>                textOnPath;
  std::optional<double>                    truncatedHeight;
  std::optional<bool>                      verticalTrim;
};

enum class PatternLayerDefClass : int
{
  PATTERN_LAYER_DEF
};

/**
 * Contains objects to define a global pattern.
 */
struct PatternLayerDef
{
  /**
   * A list of alpha masks applied to the object.
   */
  std::vector<AlphaMask>                     alphaMaskBy;
  /**
   * The bounds of the object before undergoing matrix transformations.
   */
  Rect                                       bounds;
  /**
   * A list of all child objects.
   * **Note:** The child object that appears later in the list will be displayed above the one
   * that appears first.
   */
  std::vector<Path>                          childObjects;
  PatternLayerDefClass                       patternLayerDefClass;
  /**
   * The opacity and blending-related configurations of the object.
   */
  GraphicsContextSettings                    contextSettings;
  /**
   * Smoothness of rounded corners. Range: `[0, 1]`.
   * `0` is the default value, indicating no smoothing for rounded corners.
   */
  std::optional<double>                      cornerSmoothing;
  /**
   * Horizontal constraints for the object.
   * Default value is `1`.
   */
  std::optional<int>                         horizontalConstraint;
  /**
   * ID of the object, globally unique.
   */
  std::string                                id;
  /**
   * If `true`, the object will be unable to be edited.
   */
  bool                                       isLocked;
  /**
   * `False`: When resizing occurs, the object scales according to the `horizontalConstraint`
   * and `verticalConstraint`. (Default value.)
   * `True`: When resizing occurs, the object itself maintains its angle, and the center
   * position is scaled. The scaling occurs along both the length and width directions.
   */
  std::optional<bool>                        keepShapeWhenResize;
  /**
   * How the mask object is displayed.
   */
  std::optional<int>                         maskShowType;
  /**
   * The mask type of the object.
   */
  int                                        maskType;
  /**
   * Matrix used for translating, rotating, and scaling the object.
   */
  std::vector<double>                        matrix;
  /**
   * Name of the object, for user identification, encoded in UTF-8.
   */
  std::optional<std::string>                 name;
  /**
   * A list of outline masks applied to the object, clipped by the intersection of their
   * outlines.
   * The items in the list are object IDs of the masks.
   */
  std::vector<std::string>                   outlineMaskBy;
  /**
   * How to display the child element of the object when it overflows its container.
   */
  int                                        overflow;
  /**
   * Used to be associated with the object by symbol instances for overriding its attributes.
   * Check the `objectId` in the `OverrideValue` for details.
   * If `overrideKey` exists, find a symbol master through upward traversal (which could be
   * the object itself); `overrideKey` is unique within the symbol master.
   */
  std::optional<std::string>                 overrideKey;
  std::vector<double>                        patternBoundingBox;
  /**
   * How child objects behave when the object is resized.
   */
  std::optional<int>                         resizesContent;
  /**
   * The borders, fills, and other styles of the object.
   */
  Style                                      style;
  /**
   * How the `style` of the object affects the region participating in a Boolean operation
   * with another object.
   */
  std::optional<int>                         styleEffectBoolean;
  /**
   * How the `style` and `visible` of the mask object affect the area of the mask.
   */
  int                                        styleEffectMaskArea;
  /**
   * An enclosing rectangle for the object. This property stores information about the object
   * after the matrix transformation.
   */
  std::optional<Rect>                        transformedBounds;
  /**
   * A list of variables that can be used by children.
   */
  std::optional<std::vector<VariableDefine>> variableDefs;
  /**
   * A list of referenced variables.
   */
  std::optional<std::vector<VariableRefer>>  variableRefs;
  /**
   * Vertical constraints for the object.
   * Default value is `1`.
   */
  std::optional<int>                         verticalConstraint;
  /**
   * If `false`, the object will be invisible.
   */
  bool                                       visible;
};

enum class ReferenceClass : int
{
  REFERENCED_STYLE,
  SYMBOL_MASTER
};

/**
 * A symbol master is a reusable object that contains a group of objects.
 */
struct ReferencedStyle
{
  ReferenceClass                         class_;
  std::string                            id;
  Style                                  style;
  std::optional<GraphicsContextSettings> contextSettings;
  std::optional<TextFontAttributes>      fontAttr;
};

/**
 * VGG Vector Graphics Specification is a JSON-based spec for describing vector graphics.
 *
 * In the coordinate system defined by VGG, the X-axis increases to the right, and the
 * Y-axis increases upwards. For a given coordinate point:
 * - If it is specified as normalized coordinates, it is normalized relative to the width
 * and height in the `bounds`.
 * - Otherwise, the point is non-normalized and in the same coordinate system as the object
 * `bounds`.
 */
struct DesignModel
{
  /**
   * The file name of the input design file, encoding in UTF-8.
   */
  std::optional<std::string>                  fileName;
  /**
   * The file type of current file.
   */
  int                                         fileType;
  /**
   * A list of the frames.
   */
  std::vector<Frame>                          frames;
  /**
   * Illustrator-specific pattern definitions.
   */
  std::optional<std::vector<PatternLayerDef>> patternLayerDef;
  /**
   * A list of the referenced resources.
   */
  std::optional<std::vector<ReferenceType>>   references;
  /**
   * Current VGG specs version, conforming to semantic version format like `major.minor.patch`.
   */
  std::string                                 version;
};
} // namespace Model
} // namespace VGG

namespace VGG
{
namespace Model
{
inline void from_json(const json& j, AlphaMask& x);
inline void to_json(json& j, const AlphaMask& x);

inline void from_json(const json& j, Color& x);
inline void to_json(json& j, const Color& x);

inline void from_json(const json& j, Rect& x);
inline void to_json(json& j, const Rect& x);

inline void from_json(const json& j, GraphicsContextSettings& x);
inline void to_json(json& j, const GraphicsContextSettings& x);

inline void from_json(const json& j, GradientBasicGeometry& x);
inline void to_json(json& j, const GradientBasicGeometry& x);

inline void from_json(const json& j, GradientHilight& x);
inline void to_json(json& j, const GradientHilight& x);

inline void from_json(const json& j, PerpendicularMatrix& x);
inline void to_json(json& j, const PerpendicularMatrix& x);

inline void from_json(const json& j, GradientStop& x);
inline void to_json(json& j, const GradientStop& x);

inline void from_json(const json& j, GradientInstance& x);
inline void to_json(json& j, const GradientInstance& x);

inline void from_json(const json& j, Gradient& x);
inline void to_json(json& j, const Gradient& x);

inline void from_json(const json& j, ImageFilters& x);
inline void to_json(json& j, const ImageFilters& x);

inline void from_json(const json& j, PatternImageFill& x);
inline void to_json(json& j, const PatternImageFill& x);
inline void from_json(const json& j, PatternImageStrech& x);
inline void to_json(json& j, const PatternImageStrech& x);
inline void from_json(const json& j, PatternImageFit& x);
inline void to_json(json& j, const PatternImageFit& x);
inline void from_json(const json& j, PatternImageTile& x);
inline void to_json(json& j, const PatternImageTile& x);
inline void from_json(const json& j, PatternLayerInstance& x);
inline void to_json(json& j, const PatternLayerInstance& x);

inline void from_json(const json& j, Pattern& x);
inline void to_json(json& j, const Pattern& x);

inline void from_json(const json& j, Border& x);
inline void to_json(json& j, const Border& x);

inline void from_json(const json& j, Fill& x);
inline void to_json(json& j, const Fill& x);

inline void from_json(const json& j, FontVariation& x);
inline void to_json(json& j, const FontVariation& x);

inline void from_json(const json& j, TextParagraph& x);
inline void to_json(json& j, const TextParagraph& x);

inline void from_json(const json& j, TextFontAttributes& x);
inline void to_json(json& j, const TextFontAttributes& x);

inline void from_json(const json& j, OverrideValue& x);
inline void to_json(json& j, const OverrideValue& x);

inline void from_json(const json& j, PointAttr& x);
inline void to_json(json& j, const PointAttr& x);

inline void from_json(const json& j, Region& x);
inline void to_json(json& j, const Region& x);

inline void from_json(const json& j, Segment& x);
inline void to_json(json& j, const Segment& x);

inline void from_json(const json& j, Blur& x);
inline void to_json(json& j, const Blur& x);

inline void from_json(const json& j, Shadow& x);
inline void to_json(json& j, const Shadow& x);

inline void from_json(const json& j, Style& x);
inline void to_json(json& j, const Style& x);

inline void from_json(const json& j, TextLineType& x);
inline void to_json(json& j, const TextLineType& x);

inline void from_json(const json& j, TextOnPath& x);
inline void to_json(json& j, const TextOnPath& x);

inline void from_json(const json& j, VariableAssign& x);
inline void to_json(json& j, const VariableAssign& x);

inline void from_json(const json& j, VariableDefine& x);
inline void to_json(json& j, const VariableDefine& x);

inline void from_json(const json& j, VariableRefer& x);
inline void to_json(json& j, const VariableRefer& x);

inline void from_json(const json& j, Vertex& x);
inline void to_json(json& j, const Vertex& x);

inline void from_json(const json& j, Contour& x);
inline void to_json(json& j, const Contour& x);
inline void from_json(const json& j, VectorNetwork& x);
inline void to_json(json& j, const VectorNetwork& x);
inline void from_json(const json& j, Ellipse& x);
inline void to_json(json& j, const Ellipse& x);
inline void from_json(const json& j, Polygon& x);
inline void to_json(json& j, const Polygon& x);
inline void from_json(const json& j, Rectangle& x);
inline void to_json(json& j, const Rectangle& x);
inline void from_json(const json& j, Star& x);
inline void to_json(json& j, const Star& x);

inline void from_json(const json& j, Subshape& x);
inline void to_json(json& j, const Subshape& x);

inline void from_json(const json& j, Shape& x);
inline void to_json(json& j, const Shape& x);

inline void from_json(const json& j, Object& x);
inline void to_json(json& j, const Object& x);

inline void from_json(const json& j, Container& x);
inline void to_json(json& j, const Container& x);

inline void from_json(const json& j, Path& x);
inline void to_json(json& j, const Path& x);

inline void from_json(const json& j, Frame& x);
inline void to_json(json& j, const Frame& x);

inline void from_json(const json& j, Group& x);
inline void to_json(json& j, const Group& x);

inline void from_json(const json& j, Image& x);
inline void to_json(json& j, const Image& x);

inline void from_json(const json& j, SymbolInstance& x);
inline void to_json(json& j, const SymbolInstance& x);

inline void from_json(const json& j, SymbolMaster& x);
inline void to_json(json& j, const SymbolMaster& x);

inline void from_json(const json& j, Text& x);
inline void to_json(json& j, const Text& x);

inline void from_json(const json& j, PatternLayerDef& x);
inline void to_json(json& j, const PatternLayerDef& x);

inline void from_json(const json& j, ReferencedStyle& x);
inline void to_json(json& j, const ReferencedStyle& x);

inline void from_json(const json& j, DesignModel& x);
inline void to_json(json& j, const DesignModel& x);

inline void from_json(const json& j, AlphaMaskByClass& x);
inline void to_json(json& j, const AlphaMaskByClass& x);

inline void from_json(const json& j, BackgroundColorClass& x);
inline void to_json(json& j, const BackgroundColorClass& x);

inline void from_json(const json& j, BoundsClass& x);
inline void to_json(json& j, const BoundsClass& x);

inline void from_json(const json& j, ContextSettingsClass& x);
inline void to_json(json& j, const ContextSettingsClass& x);

inline void from_json(const json& j, SubGeometryClass& x);
inline void to_json(json& j, const SubGeometryClass& x);

inline void from_json(const json& j, BorderClass& x);
inline void to_json(json& j, const BorderClass& x);

inline void from_json(const json& j, GradientClass& x);
inline void to_json(json& j, const GradientClass& x);

inline void from_json(const json& j, GeometryClass& x);
inline void to_json(json& j, const GeometryClass& x);

inline void from_json(const json& j, GradientInstanceClass& x);
inline void to_json(json& j, const GradientInstanceClass& x);

inline void from_json(const json& j, HilightClass& x);
inline void to_json(json& j, const HilightClass& x);

inline void from_json(const json& j, StopClass& x);
inline void to_json(json& j, const StopClass& x);

inline void from_json(const json& j, ImageFiltersClass& x);
inline void to_json(json& j, const ImageFiltersClass& x);

inline void from_json(const json& j, PatternInstanceClass& x);
inline void to_json(json& j, const PatternInstanceClass& x);

inline void from_json(const json& j, PatternClass& x);
inline void to_json(json& j, const PatternClass& x);

inline void from_json(const json& j, FillClass& x);
inline void to_json(json& j, const FillClass& x);

inline void from_json(const json& j, FontVariationClass& x);
inline void to_json(json& j, const FontVariationClass& x);

inline void from_json(const json& j, FontAttrClass& x);
inline void to_json(json& j, const FontAttrClass& x);

inline void from_json(const json& j, TextParagraphClass& x);
inline void to_json(json& j, const TextParagraphClass& x);

inline void from_json(const json& j, OverrideValueClass& x);
inline void to_json(json& j, const OverrideValueClass& x);

inline void from_json(const json& j, PointClass& x);
inline void to_json(json& j, const PointClass& x);

inline void from_json(const json& j, RegionClass& x);
inline void to_json(json& j, const RegionClass& x);

inline void from_json(const json& j, SegmentClass& x);
inline void to_json(json& j, const SegmentClass& x);

inline void from_json(const json& j, BlurClass& x);
inline void to_json(json& j, const BlurClass& x);

inline void from_json(const json& j, ShadowClass& x);
inline void to_json(json& j, const ShadowClass& x);

inline void from_json(const json& j, StyleClass& x);
inline void to_json(json& j, const StyleClass& x);

inline void from_json(const json& j, TextLineTypeClass& x);
inline void to_json(json& j, const TextLineTypeClass& x);

inline void from_json(const json& j, TextOnPathClass& x);
inline void to_json(json& j, const TextOnPathClass& x);

inline void from_json(const json& j, VariableAssignmentClass& x);
inline void to_json(json& j, const VariableAssignmentClass& x);

inline void from_json(const json& j, VariableDefClass& x);
inline void to_json(json& j, const VariableDefClass& x);

inline void from_json(const json& j, VariableRefClass& x);
inline void to_json(json& j, const VariableRefClass& x);

inline void from_json(const json& j, VertexClass& x);
inline void to_json(json& j, const VertexClass& x);

inline void from_json(const json& j, SubshapeClass& x);
inline void to_json(json& j, const SubshapeClass& x);

inline void from_json(const json& j, ShapeClass& x);
inline void to_json(json& j, const ShapeClass& x);

inline void from_json(const json& j, ObjectClass& x);
inline void to_json(json& j, const ObjectClass& x);

inline void from_json(const json& j, PatternLayerDefClass& x);
inline void to_json(json& j, const PatternLayerDefClass& x);

inline void from_json(const json& j, ReferenceClass& x);
inline void to_json(json& j, const ReferenceClass& x);
} // namespace Model
} // namespace VGG
namespace nlohmann
{
template<>
struct adl_serializer<std::variant<std::vector<double>, double>>
{
  static void from_json(const json& j, std::variant<std::vector<double>, double>& x);
  static void to_json(json& j, const std::variant<std::vector<double>, double>& x);
};

template<>
struct adl_serializer<VGG::Model::ContainerChildType>
{
  static void from_json(const json& j, VGG::Model::ContainerChildType& x);
  static void to_json(json& j, const VGG::Model::ContainerChildType& x);
};

template<>
struct adl_serializer<VGG::Model::SubGeometryType>
{
  static void from_json(const json& j, VGG::Model::SubGeometryType& x);
  static void to_json(json& j, const VGG::Model::SubGeometryType& x);
};

template<>
struct adl_serializer<VGG::Model::ReferenceType>
{
  static void from_json(const json& j, VGG::Model::ReferenceType& x);
  static void to_json(json& j, const VGG::Model::ReferenceType& x);
};

template<>
struct adl_serializer<VGG::Model::PatternInstanceType>
{
  static void from_json(const json& j, VGG::Model::PatternInstanceType& x);
  static void to_json(json& j, const VGG::Model::PatternInstanceType& x);
};
} // namespace nlohmann

namespace VGG
{
namespace Model
{
inline void from_json(const json& j, AlphaMask& x)
{
  safeGetTo(x.alphaType, j, "alphaType");
  safeGetTo(x.alphaMaskClass, j, "class");
  safeGetTo(x.crop, j, "crop");
  safeGetTo(x.id, j, "id");
}

inline void to_json(json& j, const AlphaMask& x)
{
  j = json::object();
  j["alphaType"] = x.alphaType;
  j["class"] = x.alphaMaskClass;
  j["crop"] = x.crop;
  j["id"] = x.id;
}

inline void from_json(const json& j, Color& x)
{
  safeGetTo(x.alpha, j, "alpha");
  safeGetTo(x.blue, j, "blue");
  safeGetTo(x.colorClass, j, "class");
  safeGetTo(x.green, j, "green");
  safeGetTo(x.red, j, "red");
}

inline void to_json(json& j, const Color& x)
{
  j = json::object();
  j["alpha"] = x.alpha;
  j["blue"] = x.blue;
  j["class"] = x.colorClass;
  j["green"] = x.green;
  j["red"] = x.red;
}

inline void from_json(const json& j, Rect& x)
{
  safeGetTo(x.rectClass, j, "class");
  safeGetTo(x.constrainProportions, j, "constrainProportions");
  safeGetTo(x.height, j, "height");
  safeGetTo(x.width, j, "width");
  safeGetTo(x.x, j, "x");
  safeGetTo(x.y, j, "y");
}

inline void to_json(json& j, const Rect& x)
{
  j = json::object();
  j["class"] = x.rectClass;
  j["constrainProportions"] = x.constrainProportions;
  j["height"] = x.height;
  j["width"] = x.width;
  j["x"] = x.x;
  j["y"] = x.y;
}

inline void from_json(const json& j, GraphicsContextSettings& x)
{
  safeGetTo(x.blendMode, j, "blendMode");
  safeGetTo(x.graphicsContextSettingsClass, j, "class");
  safeGetTo(x.isolateBlending, j, "isolateBlending");
  safeGetTo(x.opacity, j, "opacity");
  safeGetTo(x.transparencyKnockoutGroup, j, "transparencyKnockoutGroup");
}

inline void to_json(json& j, const GraphicsContextSettings& x)
{
  j = json::object();
  j["blendMode"] = x.blendMode;
  j["class"] = x.graphicsContextSettingsClass;
  j["isolateBlending"] = x.isolateBlending;
  j["opacity"] = x.opacity;
  j["transparencyKnockoutGroup"] = x.transparencyKnockoutGroup;
}

inline void from_json(const json& j, GradientBasicGeometry& x)
{
  safeGetTo(x.angle, j, "angle");
  safeGetTo(x.gradientBasicGeometryClass, j, "class");
  safeGetTo(x.flag, j, "flag");
  safeGetTo(x.length, j, "length");
  safeGetTo(x.matrix, j, "matrix");
  safeGetTo(x.widthRatio, j, "widthRatio");
  safeGetTo(x.xOrigin, j, "xOrigin");
  safeGetTo(x.yOrigin, j, "yOrigin");
}

inline void to_json(json& j, const GradientBasicGeometry& x)
{
  j = json::object();
  j["angle"] = x.angle;
  j["class"] = x.gradientBasicGeometryClass;
  j["flag"] = x.flag;
  j["length"] = x.length;
  j["matrix"] = x.matrix;
  j["widthRatio"] = x.widthRatio;
  j["xOrigin"] = x.xOrigin;
  j["yOrigin"] = x.yOrigin;
}

inline void from_json(const json& j, GradientHilight& x)
{
  safeGetTo(x.angle, j, "angle");
  safeGetTo(x.gradientHilightClass, j, "class");
  safeGetTo(x.length, j, "length");
  safeGetTo(x.xHilight, j, "xHilight");
  safeGetTo(x.yHilight, j, "yHilight");
}

inline void to_json(json& j, const GradientHilight& x)
{
  j = json::object();
  j["angle"] = x.angle;
  j["class"] = x.gradientHilightClass;
  j["length"] = x.length;
  j["xHilight"] = x.xHilight;
  j["yHilight"] = x.yHilight;
}

inline void from_json(const json& j, PerpendicularMatrix& x)
{
  safeGetTo(x.flag, j, "flag");
  safeGetTo(x.matrix, j, "matrix");
}

inline void to_json(json& j, const PerpendicularMatrix& x)
{
  j = json::object();
  j["flag"] = x.flag;
  j["matrix"] = x.matrix;
}

inline void from_json(const json& j, GradientStop& x)
{
  safeGetTo(x.gradientStopClass, j, "class");
  safeGetTo(x.color, j, "color");
  safeGetTo(x.midPoint, j, "midPoint");
  safeGetTo(x.position, j, "position");
}

inline void to_json(json& j, const GradientStop& x)
{
  j = json::object();
  j["class"] = x.gradientStopClass;
  j["color"] = x.color;
  j["midPoint"] = x.midPoint;
  j["position"] = x.position;
}

inline void from_json(const json& j, GradientInstance& x)
{
  safeGetTo(x.gradientClass, j, "class");
  safeGetTo(x.from, j, "from");
  safeGetTo(x.to, j, "to");
  safeGetTo(x.stops, j, "stops");
  x.ellipse = get_stack_optional<std::variant<std::vector<double>, double>>(j, "ellipse");
}

inline void to_json(json& j, const GradientInstance& x)
{
  j = json::object();
  j["class"] = x.gradientClass;
  j["from"] = x.from;
  j["to"] = x.to;
  j["stops"] = x.stops;
  if (x.ellipse)
  {
    j["ellipse"] = x.ellipse;
  }
}

inline void from_json(const json& j, Gradient& x)
{
  safeGetTo(x.gradientClass, j, "class");
  safeGetTo(x.instance, j, "instance");
}

inline void to_json(json& j, const Gradient& x)
{
  j = json::object();
  j["class"] = x.gradientClass;
  j["instance"] = x.instance;
}

inline void from_json(const json& j, ImageFilters& x)
{
  safeGetTo(x.imageFiltersClass, j, "class");
  x.contrast = get_stack_optional<double>(j, "contrast");
  x.exposure = get_stack_optional<double>(j, "exposure");
  x.highlights = get_stack_optional<double>(j, "highlights");
  x.hue = get_stack_optional<double>(j, "hue");
  safeGetTo(x.isEnabled, j, "isEnabled");
  x.saturation = get_stack_optional<double>(j, "saturation");
  x.shadows = get_stack_optional<double>(j, "shadows");
  x.temperature = get_stack_optional<double>(j, "temperature");
  x.tint = get_stack_optional<double>(j, "tint");
}

inline void to_json(json& j, const ImageFilters& x)
{
  j = json::object();
  j["class"] = x.imageFiltersClass;
  if (x.contrast)
  {
    j["contrast"] = x.contrast;
  }
  if (x.exposure)
  {
    j["exposure"] = x.exposure;
  }
  if (x.highlights)
  {
    j["highlights"] = x.highlights;
  }
  if (x.hue)
  {
    j["hue"] = x.hue;
  }
  j["isEnabled"] = x.isEnabled;
  if (x.saturation)
  {
    j["saturation"] = x.saturation;
  }
  if (x.shadows)
  {
    j["shadows"] = x.shadows;
  }
  if (x.temperature)
  {
    j["temperature"] = x.temperature;
  }
  if (x.tint)
  {
    j["tint"] = x.tint;
  }
}

inline void from_json(const json& j, PatternImageFill& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.imageFileName, j, "imageFileName");
  safeGetTo(x.rotation, j, "rotation");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}

inline void to_json(json& j, const PatternImageFill& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["imageFileName"] = x.imageFileName;
  j["rotation"] = x.rotation;
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
}

inline void from_json(const json& j, PatternImageStrech& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.imageFileName, j, "imageFileName");
  safeGetTo(x.matrix, j, "matrix");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}
inline void to_json(json& j, const PatternImageStrech& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["imageFileName"] = x.imageFileName;
  j["matrix"] = x.matrix;
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
}

inline void from_json(const json& j, PatternImageFit& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.imageFileName, j, "imageFileName");
  safeGetTo(x.rotation, j, "rotation");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}
inline void to_json(json& j, const PatternImageFit& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["imageFileName"] = x.imageFileName;
  j["rotation"] = x.rotation;
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
}

inline void from_json(const json& j, PatternImageTile& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.scale, j, "scale");
  safeGetTo(x.imageFileName, j, "imageFileName");
  safeGetTo(x.rotation, j, "rotation");
  x.mirror = get_stack_optional<bool>(j, "mirror");
  x.mode = get_stack_optional<int>(j, "mode");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}
inline void to_json(json& j, const PatternImageTile& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["scale"] = x.scale;
  j["imageFileName"] = x.imageFileName;
  j["rotation"] = x.rotation;
  if (x.mirror)
  {
    j["mirror"] = x.mirror;
  }
  if (x.mode)
  {
    j["mode"] = x.mode;
  }
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
}

inline void from_json(const json& j, PatternLayerInstance& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.refLayerName, j, "refLayerName");
  safeGetTo(x.offset, j, "offset");
  safeGetTo(x.scale, j, "scale");
  safeGetTo(x.angle, j, "angle");
  safeGetTo(x.reflection, j, "reflection");
  safeGetTo(x.r, j, "r");
  safeGetTo(x.shear, j, "shear");
  safeGetTo(x.shearAxis, j, "shearAxis");
  safeGetTo(x.matrix, j, "matrix");
}
inline void to_json(json& j, const PatternLayerInstance& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["refLayerName"] = x.refLayerName;
  j["offset"] = x.offset;
  j["scale"] = x.scale;
  j["angle"] = x.angle;
  j["reflection"] = x.reflection;
  j["r"] = x.r;
  j["shear"] = x.shear;
  j["shearAxis"] = x.shearAxis;
  j["matrix"] = x.matrix;
}

inline void from_json(const json& j, Pattern& x)
{
  using namespace VGG::Model;

  if (!j.is_object())
  {
    throw std::runtime_error("Could not deserialise!");
  }

  safeGetTo(x.patternClass, j, "class");
  safeGetTo(x.instance, j, "instance");
}

inline void to_json(json& j, const Pattern& x)
{
  using namespace VGG::Model;
  j = json::object();
  j["class"] = x.patternClass;
  j["instance"] = x.instance;
}

inline void from_json(const json& j, Border& x)
{
  x.borderWeightsIndependent = get_stack_optional<bool>(j, "borderWeightsIndependent");
  x.bottomWeight = get_stack_optional<double>(j, "bottomWeight");
  safeGetTo(x.borderClass, j, "class");
  x.color = get_stack_optional<Color>(j, "color");
  safeGetTo(x.contextSettings, j, "contextSettings");
  safeGetTo(x.dashedOffset, j, "dashedOffset");
  safeGetTo(x.dashedPattern, j, "dashedPattern");
  safeGetTo(x.fillType, j, "fillType");
  safeGetTo(x.flat, j, "flat");
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  safeGetTo(x.isEnabled, j, "isEnabled");
  x.leftWeight = get_stack_optional<double>(j, "leftWeight");
  safeGetTo(x.lineCapStyle, j, "lineCapStyle");
  safeGetTo(x.lineJoinStyle, j, "lineJoinStyle");
  safeGetTo(x.miterLimit, j, "miterLimit");
  x.pattern = get_stack_optional<Pattern>(j, "pattern");
  safeGetTo(x.position, j, "position");
  x.rightWeight = get_stack_optional<double>(j, "rightWeight");
  safeGetTo(x.style, j, "style");
  safeGetTo(x.thickness, j, "thickness");
  x.topWeight = get_stack_optional<double>(j, "topWeight");
}

inline void to_json(json& j, const Border& x)
{
  j = json::object();
  if (x.borderWeightsIndependent)
  {
    j["borderWeightsIndependent"] = x.borderWeightsIndependent;
  }
  if (x.bottomWeight)
  {
    j["bottomWeight"] = x.bottomWeight;
  }
  j["class"] = x.borderClass;
  if (x.color)
  {
    j["color"] = x.color;
  }
  j["contextSettings"] = x.contextSettings;
  j["dashedOffset"] = x.dashedOffset;
  j["dashedPattern"] = x.dashedPattern;
  j["fillType"] = x.fillType;
  j["flat"] = x.flat;
  if (x.gradient)
  {
    j["gradient"] = x.gradient;
  }
  j["isEnabled"] = x.isEnabled;
  if (x.leftWeight)
  {
    j["leftWeight"] = x.leftWeight;
  }
  j["lineCapStyle"] = x.lineCapStyle;
  j["lineJoinStyle"] = x.lineJoinStyle;
  j["miterLimit"] = x.miterLimit;
  if (x.pattern)
  {
    j["pattern"] = x.pattern;
  }
  j["position"] = x.position;
  if (x.rightWeight)
  {
    j["rightWeight"] = x.rightWeight;
  }
  j["style"] = x.style;
  j["thickness"] = x.thickness;
  if (x.topWeight)
  {
    j["topWeight"] = x.topWeight;
  }
}

inline void from_json(const json& j, Fill& x)
{
  safeGetTo(x.fillClass, j, "class");
  x.color = get_stack_optional<Color>(j, "color");
  safeGetTo(x.contextSettings, j, "contextSettings");
  safeGetTo(x.fillType, j, "fillType");
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  safeGetTo(x.isEnabled, j, "isEnabled");
  x.pattern = get_stack_optional<Pattern>(j, "pattern");
}

inline void to_json(json& j, const Fill& x)
{
  j = json::object();
  j["class"] = x.fillClass;
  if (x.color)
  {
    j["color"] = x.color;
  }
  j["contextSettings"] = x.contextSettings;
  j["fillType"] = x.fillType;
  if (x.gradient)
  {
    j["gradient"] = x.gradient;
  }
  j["isEnabled"] = x.isEnabled;
  if (x.pattern)
  {
    j["pattern"] = x.pattern;
  }
}

inline void from_json(const json& j, FontVariation& x)
{
  safeGetTo(x.fontVariationClass, j, "class");
  safeGetTo(x.name, j, "name");
  safeGetTo(x.value, j, "value");
}

inline void to_json(json& j, const FontVariation& x)
{
  j = json::object();
  j["class"] = x.fontVariationClass;
  j["name"] = x.name;
  j["value"] = x.value;
}

inline void from_json(const json& j, TextParagraph& x)
{
  safeGetTo(x.textParagraphClass, j, "class");
  safeGetTo(x.paragraphSpacing, j, "paragraphSpacing");
}

inline void to_json(json& j, const TextParagraph& x)
{
  j = json::object();
  j["class"] = x.textParagraphClass;
  j["paragraphSpacing"] = x.paragraphSpacing;
}

inline void from_json(const json& j, TextFontAttributes& x)
{
  x.baselineShift = get_stack_optional<double>(j, "baselineShift");
  x.borders = get_stack_optional<std::vector<Border>>(j, "borders");
  safeGetTo(x.textFontAttributesClass, j, "class");
  x.fills = get_stack_optional<std::vector<Fill>>(j, "fills");
  x.fillUseType = get_stack_optional<int>(j, "fillUseType");
  x.fontVariantCaps = get_stack_optional<int>(j, "fontVariantCaps");
  x.fontVariantPosition = get_stack_optional<int>(j, "fontVariantPosition");
  x.fontVariations = get_stack_optional<std::vector<FontVariation>>(j, "fontVariations");
  x.horizontalScale = get_stack_optional<double>(j, "horizontalScale");
  x.hyperlink = get_stack_optional<std::string>(j, "hyperlink");
  x.length = get_stack_optional<int>(j, "length");
  x.letterSpacingUnit = get_stack_optional<int>(j, "letterSpacingUnit");
  x.letterSpacingValue = get_stack_optional<double>(j, "letterSpacingValue");
  x.lineSpacingUnit = get_stack_optional<int>(j, "lineSpacingUnit");
  x.lineSpacingValue = get_stack_optional<double>(j, "lineSpacingValue");
  x.linethrough = get_stack_optional<bool>(j, "linethrough");
  x.name = get_stack_optional<std::string>(j, "name");
  x.postScript = get_stack_optional<std::string>(j, "postScript");
  x.rotate = get_stack_optional<double>(j, "rotate");
  x.size = get_stack_optional<double>(j, "size");
  x.subFamilyName = get_stack_optional<std::string>(j, "subFamilyName");
  x.textCase = get_stack_optional<int>(j, "textCase");
  x.textParagraph = get_stack_optional<TextParagraph>(j, "textParagraph");
  x.underline = get_stack_optional<int>(j, "underline");
  x.verticalScale = get_stack_optional<double>(j, "verticalScale");
}

inline void to_json(json& j, const TextFontAttributes& x)
{
  j = json::object();
  if (x.baselineShift)
  {
    j["baselineShift"] = x.baselineShift;
  }
  if (x.borders)
  {
    j["borders"] = x.borders;
  }
  j["class"] = x.textFontAttributesClass;
  if (x.fills)
  {
    j["fills"] = x.fills;
  }
  if (x.fillUseType)
  {
    j["fillUseType"] = x.fillUseType;
  }
  if (x.fontVariantCaps)
  {
    j["fontVariantCaps"] = x.fontVariantCaps;
  }
  if (x.fontVariantPosition)
  {
    j["fontVariantPosition"] = x.fontVariantPosition;
  }
  if (x.fontVariations)
  {
    j["fontVariations"] = x.fontVariations;
  }
  if (x.horizontalScale)
  {
    j["horizontalScale"] = x.horizontalScale;
  }
  if (x.hyperlink)
  {
    j["hyperlink"] = x.hyperlink;
  }
  if (x.length)
  {
    j["length"] = x.length;
  }
  if (x.letterSpacingUnit)
  {
    j["letterSpacingUnit"] = x.letterSpacingUnit;
  }
  if (x.letterSpacingValue)
  {
    j["letterSpacingValue"] = x.letterSpacingValue;
  }
  if (x.lineSpacingUnit)
  {
    j["lineSpacingUnit"] = x.lineSpacingUnit;
  }
  if (x.lineSpacingValue)
  {
    j["lineSpacingValue"] = x.lineSpacingValue;
  }
  if (x.linethrough)
  {
    j["linethrough"] = x.linethrough;
  }
  if (x.name)
  {
    j["name"] = x.name;
  }
  if (x.postScript)
  {
    j["postScript"] = x.postScript;
  }
  if (x.rotate)
  {
    j["rotate"] = x.rotate;
  }
  if (x.size)
  {
    j["size"] = x.size;
  }
  if (x.subFamilyName)
  {
    j["subFamilyName"] = x.subFamilyName;
  }
  if (x.textCase)
  {
    j["textCase"] = x.textCase;
  }
  if (x.textParagraph)
  {
    j["textParagraph"] = x.textParagraph;
  }
  if (x.underline)
  {
    j["underline"] = x.underline;
  }
  if (x.verticalScale)
  {
    j["verticalScale"] = x.verticalScale;
  }
}

inline void from_json(const json& j, OverrideValue& x)
{
  safeGetTo(x.overrideValueClass, j, "class");
  x.effectOnLayout = get_stack_optional<bool>(j, "effectOnLayout");
  safeGetTo(x.objectId, j, "objectId");
  safeGetTo(x.overrideName, j, "overrideName");
  x.overrideValue = get_untyped(j, "overrideValue");
}

inline void to_json(json& j, const OverrideValue& x)
{
  j = json::object();
  j["class"] = x.overrideValueClass;
  if (x.effectOnLayout)
  {
    j["effectOnLayout"] = x.effectOnLayout;
  }
  j["objectId"] = x.objectId;
  j["overrideName"] = x.overrideName;
  j["overrideValue"] = x.overrideValue;
}

inline void from_json(const json& j, PointAttr& x)
{
  safeGetTo(x.pointAttrClass, j, "class");
  x.cornerStyle = get_stack_optional<int>(j, "cornerStyle");
  x.curveFrom = get_stack_optional<std::vector<double>>(j, "curveFrom");
  x.curveTo = get_stack_optional<std::vector<double>>(j, "curveTo");
  x.markType = get_stack_optional<int>(j, "markType");
  safeGetTo(x.point, j, "point");
  x.radius = get_stack_optional<double>(j, "radius");
}

inline void to_json(json& j, const PointAttr& x)
{
  j = json::object();
  j["class"] = x.pointAttrClass;
  if (x.cornerStyle)
  {
    j["cornerStyle"] = x.cornerStyle;
  }
  if (x.curveFrom)
  {
    j["curveFrom"] = x.curveFrom;
  }
  if (x.curveTo)
  {
    j["curveTo"] = x.curveTo;
  }
  if (x.markType)
  {
    j["markType"] = x.markType;
  }
  j["point"] = x.point;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Region& x)
{
  safeGetTo(x.regionClass, j, "class");
  x.fills = get_stack_optional<std::vector<Fill>>(j, "fills");
  safeGetTo(x.loops, j, "loops");
  safeGetTo(x.windingRule, j, "windingRule");
}

inline void to_json(json& j, const Region& x)
{
  j = json::object();
  j["class"] = x.regionClass;
  if (x.fills)
  {
    j["fills"] = x.fills;
  }
  j["loops"] = x.loops;
  j["windingRule"] = x.windingRule;
}

inline void from_json(const json& j, Segment& x)
{
  safeGetTo(x.segmentClass, j, "class");
  x.curveFrom = get_stack_optional<std::vector<double>>(j, "curveFrom");
  x.curveTo = get_stack_optional<std::vector<double>>(j, "curveTo");
  safeGetTo(x.end, j, "end");
  safeGetTo(x.start, j, "start");
}

inline void to_json(json& j, const Segment& x)
{
  j = json::object();
  j["class"] = x.segmentClass;
  if (x.curveFrom)
  {
    j["curveFrom"] = x.curveFrom;
  }
  if (x.curveTo)
  {
    j["curveTo"] = x.curveTo;
  }
  j["end"] = x.end;
  j["start"] = x.start;
}

inline void from_json(const json& j, Blur& x)
{
  safeGetTo(x.center, j, "center");
  safeGetTo(x.blurClass, j, "class");
  safeGetTo(x.isEnabled, j, "isEnabled");
  x.motionAngle = get_stack_optional<double>(j, "motionAngle");
  x.radius = get_stack_optional<double>(j, "radius");
  safeGetTo(x.saturation, j, "saturation");
  safeGetTo(x.type, j, "type");
}

inline void to_json(json& j, const Blur& x)
{
  j = json::object();
  j["center"] = x.center;
  j["class"] = x.blurClass;
  j["isEnabled"] = x.isEnabled;
  if (x.motionAngle)
  {
    j["motionAngle"] = x.motionAngle;
  }
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
  j["saturation"] = x.saturation;
  j["type"] = x.type;
}

inline void from_json(const json& j, Shadow& x)
{
  safeGetTo(x.blur, j, "blur");
  safeGetTo(x.shadowClass, j, "class");
  safeGetTo(x.color, j, "color");
  safeGetTo(x.contextSettings, j, "contextSettings");
  safeGetTo(x.inner, j, "inner");
  safeGetTo(x.isEnabled, j, "isEnabled");
  safeGetTo(x.offsetX, j, "offsetX");
  safeGetTo(x.offsetY, j, "offsetY");
  x.showBehindTransparentAreas = get_stack_optional<bool>(j, "showBehindTransparentAreas");
  safeGetTo(x.spread, j, "spread");
}

inline void to_json(json& j, const Shadow& x)
{
  j = json::object();
  j["blur"] = x.blur;
  j["class"] = x.shadowClass;
  j["color"] = x.color;
  j["contextSettings"] = x.contextSettings;
  j["inner"] = x.inner;
  j["isEnabled"] = x.isEnabled;
  j["offsetX"] = x.offsetX;
  j["offsetY"] = x.offsetY;
  if (x.showBehindTransparentAreas)
  {
    j["showBehindTransparentAreas"] = x.showBehindTransparentAreas;
  }
  j["spread"] = x.spread;
}

inline void from_json(const json& j, Style& x)
{
  safeGetTo(x.blurs, j, "blurs");
  safeGetTo(x.borders, j, "borders");
  safeGetTo(x.styleClass, j, "class");
  safeGetTo(x.fills, j, "fills");
  safeGetTo(x.shadows, j, "shadows");
}

inline void to_json(json& j, const Style& x)
{
  j = json::object();
  j["blurs"] = x.blurs;
  j["borders"] = x.borders;
  j["class"] = x.styleClass;
  j["fills"] = x.fills;
  j["shadows"] = x.shadows;
}

inline void from_json(const json& j, TextLineType& x)
{
  safeGetTo(x.textLineTypeClass, j, "class");
  safeGetTo(x.isFirst, j, "isFirst");
  safeGetTo(x.level, j, "level");
  safeGetTo(x.styleType, j, "styleType");
}

inline void to_json(json& j, const TextLineType& x)
{
  j = json::object();
  j["class"] = x.textLineTypeClass;
  j["isFirst"] = x.isFirst;
  j["level"] = x.level;
  j["styleType"] = x.styleType;
}

inline void from_json(const json& j, TextOnPath& x)
{
  safeGetTo(x.textOnPathClass, j, "class");
}

inline void to_json(json& j, const TextOnPath& x)
{
  j = json::object();
  j["class"] = x.textOnPathClass;
}

inline void from_json(const json& j, VariableAssign& x)
{
  safeGetTo(x.variableAssignClass, j, "class");
  safeGetTo(x.id, j, "id");
  x.value = get_untyped(j, "value");
}

inline void to_json(json& j, const VariableAssign& x)
{
  j = json::object();
  j["class"] = x.variableAssignClass;
  j["id"] = x.id;
  j["value"] = x.value;
}

inline void from_json(const json& j, VariableDefine& x)
{
  safeGetTo(x.variableDefineClass, j, "class");
  safeGetTo(x.id, j, "id");
  x.value = get_untyped(j, "value");
  safeGetTo(x.varType, j, "varType");
}

inline void to_json(json& j, const VariableDefine& x)
{
  j = json::object();
  j["class"] = x.variableDefineClass;
  j["id"] = x.id;
  j["value"] = x.value;
  j["varType"] = x.varType;
}

inline void from_json(const json& j, VariableRefer& x)
{
  safeGetTo(x.variableReferClass, j, "class");
  safeGetTo(x.id, j, "id");
  safeGetTo(x.objectField, j, "objectField");
}

inline void to_json(json& j, const VariableRefer& x)
{
  j = json::object();
  j["class"] = x.variableReferClass;
  j["id"] = x.id;
  j["objectField"] = x.objectField;
}

inline void from_json(const json& j, Vertex& x)
{
  safeGetTo(x.vertexClass, j, "class");
  x.markType = get_stack_optional<int>(j, "markType");
  safeGetTo(x.point, j, "point");
  x.radius = get_stack_optional<double>(j, "radius");
}

inline void to_json(json& j, const Vertex& x)
{
  j = json::object();
  j["class"] = x.vertexClass;
  if (x.markType)
  {
    j["markType"] = x.markType;
  }
  j["point"] = x.point;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Contour& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.closed, j, "closed");
  safeGetTo(x.points, j, "points");
}

inline void to_json(json& j, const Contour& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["closed"] = x.closed;
  j["points"] = x.points;
}

inline void from_json(const json& j, VectorNetwork& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.vertices, j, "vertices");
  safeGetTo(x.segments, j, "segments");
  safeGetTo(x.regions, j, "regions");
}
inline void to_json(json& j, const VectorNetwork& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["vertices"] = x.vertices;
  j["segments"] = x.segments;
  j["regions"] = x.regions;
}

inline void from_json(const json& j, Ellipse& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.startingAngle, j, "startingAngle");
  safeGetTo(x.endingAngle, j, "endingAngle");
  safeGetTo(x.innerRadius, j, "innerRadius");
}
inline void to_json(json& j, const Ellipse& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["startingAngle"] = x.startingAngle;
  j["endingAngle"] = x.endingAngle;
  j["innerRadius"] = x.innerRadius;
}

inline void from_json(const json& j, Polygon& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.pointCount, j, "pointCount");
  x.radius = get_stack_optional<double>(j, "radius");
}
inline void to_json(json& j, const Polygon& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["pointCount"] = x.pointCount;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Rectangle& x)
{
  safeGetTo(x.class_, j, "class");
  x.cornerRadius = get_stack_optional<double>(j, "cornerRadius");
  x.radius = get_stack_optional<std::vector<double>>(j, "radius");
}
inline void to_json(json& j, const Rectangle& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["cornerRadius"] = x.cornerRadius;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Star& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.ratio, j, "ratio");
  safeGetTo(x.pointCount, j, "pointCount");
  x.radius = get_stack_optional<double>(j, "radius");
}
inline void to_json(json& j, const Star& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["ratio"] = x.ratio;
  j["pointCount"] = x.pointCount;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Subshape& x)
{
  safeGetTo(x.booleanOperation, j, "booleanOperation");
  safeGetTo(x.subshapeClass, j, "class");

  safeGetTo(x.subGeometry, j, "subGeometry");
}

inline void to_json(json& j, const Subshape& x)
{
  j = json::object();
  j["booleanOperation"] = x.booleanOperation;
  j["class"] = x.subshapeClass;
  j["subGeometry"] = x.subGeometry;
}

inline void from_json(const json& j, Shape& x)
{
  safeGetTo(x.shapeClass, j, "class");
  x.radius = get_stack_optional<double>(j, "radius");
  safeGetTo(x.subshapes, j, "subshapes");
  safeGetTo(x.windingRule, j, "windingRule");
}

inline void to_json(json& j, const Shape& x)
{
  j = json::object();
  j["class"] = x.shapeClass;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
  j["subshapes"] = x.subshapes;
  j["windingRule"] = x.windingRule;
}

inline void from_json(const json& j, Object& x)
{
  safeGetTo(x.class_, j, "class");

  safeGetTo(x.alphaMaskBy, j, "alphaMaskBy");
  safeGetTo(x.bounds, j, "bounds");
  safeGetTo(x.contextSettings, j, "contextSettings");
  safeGetTo(x.id, j, "id");
  safeGetTo(x.isLocked, j, "isLocked");
  safeGetTo(x.maskType, j, "maskType");
  safeGetTo(x.matrix, j, "matrix");
  safeGetTo(x.outlineMaskBy, j, "outlineMaskBy");
  safeGetTo(x.overflow, j, "overflow");
  safeGetTo(x.style, j, "style");
  safeGetTo(x.styleEffectMaskArea, j, "styleEffectMaskArea");
  safeGetTo(x.visible, j, "visible");

  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int>(j, "horizontalConstraint");
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int>(j, "maskShowType");
  x.name = get_stack_optional<std::string>(j, "name");
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.resizesContent = get_stack_optional<int>(j, "resizesContent");
  x.styleEffectBoolean = get_stack_optional<int>(j, "styleEffectBoolean");
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int>(j, "verticalConstraint");
}
inline void to_json(json& j, const Object& x)
{
  j = json::object();
  j["class"] = x.class_;

  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["contextSettings"] = x.contextSettings;
  j["id"] = x.id;
  j["isLocked"] = x.isLocked;
  j["maskType"] = x.maskType;
  j["matrix"] = x.matrix;
  j["outlineMaskBy"] = x.outlineMaskBy;
  j["overflow"] = x.overflow;
  j["style"] = x.style;
  j["styleEffectMaskArea"] = x.styleEffectMaskArea;
  j["visible"] = x.visible;

  if (x.cornerSmoothing)
  {
    j["cornerSmoothing"] = x.cornerSmoothing;
  }
  if (x.horizontalConstraint)
  {
    j["horizontalConstraint"] = x.horizontalConstraint;
  }
  if (x.keepShapeWhenResize)
  {
    j["keepShapeWhenResize"] = x.keepShapeWhenResize;
  }
  if (x.maskShowType)
  {
    j["maskShowType"] = x.maskShowType;
  }
  if (x.name)
  {
    j["name"] = x.name;
  }
  if (x.overrideKey)
  {
    j["overrideKey"] = x.overrideKey;
  }
  if (x.resizesContent)
  {
    j["resizesContent"] = x.resizesContent;
  }
  if (x.styleEffectBoolean)
  {
    j["styleEffectBoolean"] = x.styleEffectBoolean;
  }
  if (x.transformedBounds)
  {
    j["transformedBounds"] = x.transformedBounds;
  }
  if (x.variableDefs)
  {
    j["variableDefs"] = x.variableDefs;
  }
  if (x.variableRefs)
  {
    j["variableRefs"] = x.variableRefs;
  }
  if (x.verticalConstraint)
  {
    j["verticalConstraint"] = x.verticalConstraint;
  }
}

inline void from_json(const json& j, Container& x)
{
  from_json(j, static_cast<Object&>(x));
  safeGetTo(x.childObjects, j, "childObjects");
}
inline void to_json(json& j, const Container& x)
{
  to_json(j, static_cast<const Object&>(x));
  j["childObjects"] = x.childObjects;
}

inline void from_json(const json& j, Path& x)
{
  from_json(j, static_cast<Object&>(x));

  safeGetTo(x.shape, j, "shape");
}
inline void to_json(json& j, const Path& x)
{
  to_json(j, static_cast<const Object&>(x));

  j["shape"] = x.shape;
}

inline void from_json(const json& j, Frame& x)
{
  from_json(j, static_cast<Container&>(x));
  x.backgroundColor = get_stack_optional<Color>(j, "backgroundColor");
  x.radius = get_stack_optional<std::vector<double>>(j, "radius");
}

inline void to_json(json& j, const Frame& x)
{
  to_json(j, static_cast<const Container&>(x));
  if (x.backgroundColor)
  {
    j["backgroundColor"] = x.backgroundColor;
  }
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Group& x)
{
  from_json(j, static_cast<Container&>(x));
  x.groupNestMaskType = get_stack_optional<bool>(j, "groupNestMaskType");
  x.isVectorNetwork = get_stack_optional<bool>(j, "isVectorNetwork");
}

inline void to_json(json& j, const Group& x)
{
  to_json(j, static_cast<const Container&>(x));
  if (x.groupNestMaskType)
  {
    j["groupNestMaskType"] = x.groupNestMaskType;
  }
  if (x.isVectorNetwork)
  {
    j["isVectorNetwork"] = x.isVectorNetwork;
  }
}

inline void from_json(const json& j, Image& x)
{
  from_json(j, static_cast<Object&>(x));

  safeGetTo(x.imageFileName, j, "imageFileName");

  x.fillReplacesImage = get_stack_optional<bool>(j, "fillReplacesImage");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}
inline void to_json(json& j, const Image& x)
{
  to_json(j, static_cast<const Object&>(x));

  j["imageFileName"] = x.imageFileName;

  if (x.fillReplacesImage)
  {
    j["fillReplacesImage"] = x.fillReplacesImage;
  }
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
}

inline void from_json(const json& j, SymbolInstance& x)
{
  from_json(j, static_cast<Object&>(x));

  safeGetTo(x.masterId, j, "masterId");
  safeGetTo(x.overrideValues, j, "overrideValues");

  x.radius = get_stack_optional<std::vector<double>>(j, "radius");
  x.variableAssignments = get_stack_optional<std::vector<VariableAssign>>(j, "variableAssignments");
}
inline void to_json(json& j, const SymbolInstance& x)
{
  to_json(j, static_cast<const Object&>(x));

  j["masterId"] = x.masterId;
  j["overrideValues"] = x.overrideValues;

  if (x.radius)
  {
    j["radius"] = x.radius;
  }
  if (x.variableAssignments)
  {
    j["variableAssignments"] = x.variableAssignments;
  }
}

inline void from_json(const json& j, SymbolMaster& x)
{
  from_json(j, static_cast<Container&>(x));
}
inline void to_json(json& j, const SymbolMaster& x)
{
  to_json(j, static_cast<const Container&>(x));
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

inline void from_json(const json& j, Text& x)
{
  from_json(j, static_cast<Object&>(x));

  safeGetTo(x.content, j, "content");
  safeGetTo(x.fontAttr, j, "fontAttr");
  safeGetTo(x.frameMode, j, "frameMode");
  safeGetTo(x.horizontalAlignment, j, "horizontalAlignment");
  safeGetTo(x.verticalAlignment, j, "verticalAlignment");

  x.anchorPoint = get_stack_optional<std::vector<double>>(j, "anchorPoint");
  x.defaultFontAttr = get_stack_optional<TextFontAttributes>(j, "defaultFontAttr");
  x.textLineType = get_stack_optional<std::vector<TextLineType>>(j, "textLineType");
  x.textOnPath = get_stack_optional<TextOnPath>(j, "textOnPath");
  x.truncatedHeight = get_stack_optional<double>(j, "truncatedHeight");
  x.verticalTrim = get_stack_optional<bool>(j, "verticalTrim");
}
inline void to_json(json& j, const Text& x)
{
  to_json(j, static_cast<const Object&>(x));

  j["content"] = x.content;
  j["fontAttr"] = x.fontAttr;
  j["frameMode"] = x.frameMode;
  j["horizontalAlignment"] = x.horizontalAlignment;
  j["verticalAlignment"] = x.verticalAlignment;

  if (x.anchorPoint)
  {
    j["anchorPoint"] = x.anchorPoint;
  }
  if (x.defaultFontAttr)
  {
    j["defaultFontAttr"] = x.defaultFontAttr;
  }
  if (x.textLineType)
  {
    j["textLineType"] = x.textLineType;
  }
  if (x.textOnPath)
  {
    j["textOnPath"] = x.textOnPath;
  }
  if (x.truncatedHeight)
  {
    j["truncatedHeight"] = x.truncatedHeight;
  }
  if (x.verticalTrim)
  {
    j["verticalTrim"] = x.verticalTrim;
  }
}

inline void from_json(const json& j, PatternLayerDef& x)
{
  safeGetTo(x.alphaMaskBy, j, "alphaMaskBy");
  safeGetTo(x.bounds, j, "bounds");
  safeGetTo(x.childObjects, j, "childObjects");
  safeGetTo(x.patternLayerDefClass, j, "class");
  safeGetTo(x.contextSettings, j, "contextSettings");
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int>(j, "horizontalConstraint");
  safeGetTo(x.id, j, "id");
  safeGetTo(x.isLocked, j, "isLocked");
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int>(j, "maskShowType");
  safeGetTo(x.maskType, j, "maskType");
  safeGetTo(x.matrix, j, "matrix");
  x.name = get_stack_optional<std::string>(j, "name");
  safeGetTo(x.outlineMaskBy, j, "outlineMaskBy");
  safeGetTo(x.overflow, j, "overflow");
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  safeGetTo(x.patternBoundingBox, j, "patternBoundingBox");
  x.resizesContent = get_stack_optional<int>(j, "resizesContent");
  safeGetTo(x.style, j, "style");
  x.styleEffectBoolean = get_stack_optional<int>(j, "styleEffectBoolean");
  safeGetTo(x.styleEffectMaskArea, j, "styleEffectMaskArea");
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int>(j, "verticalConstraint");
  safeGetTo(x.visible, j, "visible");
}

inline void to_json(json& j, const PatternLayerDef& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["childObjects"] = x.childObjects;
  j["class"] = x.patternLayerDefClass;
  j["contextSettings"] = x.contextSettings;
  if (x.cornerSmoothing)
  {
    j["cornerSmoothing"] = x.cornerSmoothing;
  }
  if (x.horizontalConstraint)
  {
    j["horizontalConstraint"] = x.horizontalConstraint;
  }
  j["id"] = x.id;
  j["isLocked"] = x.isLocked;
  if (x.keepShapeWhenResize)
  {
    j["keepShapeWhenResize"] = x.keepShapeWhenResize;
  }
  if (x.maskShowType)
  {
    j["maskShowType"] = x.maskShowType;
  }
  j["maskType"] = x.maskType;
  j["matrix"] = x.matrix;
  if (x.name)
  {
    j["name"] = x.name;
  }
  j["outlineMaskBy"] = x.outlineMaskBy;
  j["overflow"] = x.overflow;
  if (x.overrideKey)
  {
    j["overrideKey"] = x.overrideKey;
  }
  j["patternBoundingBox"] = x.patternBoundingBox;
  if (x.resizesContent)
  {
    j["resizesContent"] = x.resizesContent;
  }
  j["style"] = x.style;
  if (x.styleEffectBoolean)
  {
    j["styleEffectBoolean"] = x.styleEffectBoolean;
  }
  j["styleEffectMaskArea"] = x.styleEffectMaskArea;
  if (x.transformedBounds)
  {
    j["transformedBounds"] = x.transformedBounds;
  }
  if (x.variableDefs)
  {
    j["variableDefs"] = x.variableDefs;
  }
  if (x.variableRefs)
  {
    j["variableRefs"] = x.variableRefs;
  }
  if (x.verticalConstraint)
  {
    j["verticalConstraint"] = x.verticalConstraint;
  }
  j["visible"] = x.visible;
}

inline void from_json(const json& j, ReferencedStyle& x)
{
  safeGetTo(x.class_, j, "class");
  safeGetTo(x.id, j, "id");
  safeGetTo(x.style, j, "style");
  x.contextSettings = get_stack_optional<GraphicsContextSettings>(j, "contextSettings");
  x.fontAttr = get_stack_optional<TextFontAttributes>(j, "fontAttr");
}

inline void to_json(json& j, const ReferencedStyle& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["id"] = x.id;
  j["style"] = x.style;
  if (x.contextSettings)
  {
    j["contextSettings"] = x.contextSettings;
  }
  if (x.fontAttr)
  {
    j["fontAttr"] = x.fontAttr;
  }
}

inline void from_json(const json& j, DesignModel& x)
{
  x.fileName = get_stack_optional<std::string>(j, "fileName");
  safeGetTo(x.fileType, j, "fileType");
  safeGetTo(x.frames, j, "frames");
  x.patternLayerDef = get_stack_optional<std::vector<PatternLayerDef>>(j, "patternLayerDef");
  x.references = get_stack_optional<std::vector<ReferenceType>>(j, "references");
  safeGetTo(x.version, j, "version");
}

inline void to_json(json& j, const DesignModel& x)
{
  j = json::object();
  if (x.fileName)
  {
    j["fileName"] = x.fileName;
  }
  j["fileType"] = x.fileType;
  j["frames"] = x.frames;
  if (x.patternLayerDef)
  {
    j["patternLayerDef"] = x.patternLayerDef;
  }
  if (x.references)
  {
    j["references"] = x.references;
  }
  j["version"] = x.version;
}

inline void from_json(const json& j, AlphaMaskByClass& x)
{
  if (j == "alphaMask")
    x = AlphaMaskByClass::ALPHA_MASK;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const AlphaMaskByClass& x)
{
  switch (x)
  {
    case AlphaMaskByClass::ALPHA_MASK:
      j = "alphaMask";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, BackgroundColorClass& x)
{
  if (j == "color")
    x = BackgroundColorClass::COLOR;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const BackgroundColorClass& x)
{
  switch (x)
  {
    case BackgroundColorClass::COLOR:
      j = "color";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, BoundsClass& x)
{
  if (j == "rect")
    x = BoundsClass::RECT;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const BoundsClass& x)
{
  switch (x)
  {
    case BoundsClass::RECT:
      j = "rect";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, ContextSettingsClass& x)
{
  if (j == "graphicsContextSettings")
    x = ContextSettingsClass::GRAPHICS_CONTEXT_SETTINGS;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ContextSettingsClass& x)
{
  switch (x)
  {
    case ContextSettingsClass::GRAPHICS_CONTEXT_SETTINGS:
      j = "graphicsContextSettings";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, SubGeometryClass& x)
{
  if (j == "contour")
    x = SubGeometryClass::CONTOUR;
  else if (j == "ellipse")
    x = SubGeometryClass::ELLIPSE;
  else if (j == "frame")
    x = SubGeometryClass::FRAME;
  else if (j == "group")
    x = SubGeometryClass::GROUP;
  else if (j == "image")
    x = SubGeometryClass::IMAGE;
  else if (j == "path")
    x = SubGeometryClass::PATH;
  else if (j == "polygon")
    x = SubGeometryClass::POLYGON;
  else if (j == "rectangle")
    x = SubGeometryClass::RECTANGLE;
  else if (j == "star")
    x = SubGeometryClass::STAR;
  else if (j == "symbolInstance")
    x = SubGeometryClass::SYMBOL_INSTANCE;
  else if (j == "symbolMaster")
    x = SubGeometryClass::SYMBOL_MASTER;
  else if (j == "text")
    x = SubGeometryClass::TEXT;
  else if (j == "vectorNetwork")
    x = SubGeometryClass::VECTOR_NETWORK;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const SubGeometryClass& x)
{
  switch (x)
  {
    case SubGeometryClass::CONTOUR:
      j = "contour";
      break;
    case SubGeometryClass::ELLIPSE:
      j = "ellipse";
      break;
    case SubGeometryClass::FRAME:
      j = "frame";
      break;
    case SubGeometryClass::GROUP:
      j = "group";
      break;
    case SubGeometryClass::IMAGE:
      j = "image";
      break;
    case SubGeometryClass::PATH:
      j = "path";
      break;
    case SubGeometryClass::POLYGON:
      j = "polygon";
      break;
    case SubGeometryClass::RECTANGLE:
      j = "rectangle";
      break;
    case SubGeometryClass::STAR:
      j = "star";
      break;
    case SubGeometryClass::SYMBOL_INSTANCE:
      j = "symbolInstance";
      break;
    case SubGeometryClass::SYMBOL_MASTER:
      j = "symbolMaster";
      break;
    case SubGeometryClass::TEXT:
      j = "text";
      break;
    case SubGeometryClass::VECTOR_NETWORK:
      j = "vectorNetwork";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, BorderClass& x)
{
  if (j == "border")
    x = BorderClass::BORDER;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const BorderClass& x)
{
  switch (x)
  {
    case BorderClass::BORDER:
      j = "border";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, GradientClass& x)
{
  if (j == "gradient")
    x = GradientClass::GRADIENT;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const GradientClass& x)
{
  switch (x)
  {
    case GradientClass::GRADIENT:
      j = "gradient";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, GeometryClass& x)
{
  if (j == "gradientBasicGeometry")
    x = GeometryClass::GRADIENT_BASIC_GEOMETRY;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const GeometryClass& x)
{
  switch (x)
  {
    case GeometryClass::GRADIENT_BASIC_GEOMETRY:
      j = "gradientBasicGeometry";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, GradientInstanceClass& x)
{
  if (j == "gradientAngular")
    x = GradientInstanceClass::GRADIENT_ANGULAR;
  else if (j == "gradientBasic")
    x = GradientInstanceClass::GRADIENT_BASIC;
  else if (j == "gradientDiamond")
    x = GradientInstanceClass::GRADIENT_DIAMOND;
  else if (j == "gradientLinear")
    x = GradientInstanceClass::GRADIENT_LINEAR;
  else if (j == "gradientRadial")
    x = GradientInstanceClass::GRADIENT_RADIAL;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const GradientInstanceClass& x)
{
  switch (x)
  {
    case GradientInstanceClass::GRADIENT_ANGULAR:
      j = "gradientAngular";
      break;
    case GradientInstanceClass::GRADIENT_BASIC:
      j = "gradientBasic";
      break;
    case GradientInstanceClass::GRADIENT_DIAMOND:
      j = "gradientDiamond";
      break;
    case GradientInstanceClass::GRADIENT_LINEAR:
      j = "gradientLinear";
      break;
    case GradientInstanceClass::GRADIENT_RADIAL:
      j = "gradientRadial";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, HilightClass& x)
{
  if (j == "gradientHilight")
    x = HilightClass::GRADIENT_HILIGHT;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const HilightClass& x)
{
  switch (x)
  {
    case HilightClass::GRADIENT_HILIGHT:
      j = "gradientHilight";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, StopClass& x)
{
  if (j == "gradientStop")
    x = StopClass::GRADIENT_STOP;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const StopClass& x)
{
  switch (x)
  {
    case StopClass::GRADIENT_STOP:
      j = "gradientStop";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, ImageFiltersClass& x)
{
  if (j == "imageFilters")
    x = ImageFiltersClass::IMAGE_FILTERS;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ImageFiltersClass& x)
{
  switch (x)
  {
    case ImageFiltersClass::IMAGE_FILTERS:
      j = "imageFilters";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, PatternInstanceClass& x)
{
  if (j == "patternImageFill")
    x = PatternInstanceClass::PATTERN_IMAGE_FILL;
  else if (j == "patternImageFit")
    x = PatternInstanceClass::PATTERN_IMAGE_FIT;
  else if (j == "patternImageStretch")
    x = PatternInstanceClass::PATTERN_IMAGE_STRETCH;
  else if (j == "patternImageTile")
    x = PatternInstanceClass::PATTERN_IMAGE_TILE;
  else if (j == "patternLayer")
    x = PatternInstanceClass::PATTERN_LAYER;
  else
  {
    x = PatternInstanceClass::PATTERN_UNKNOWN;
  }
}

inline void to_json(json& j, const PatternInstanceClass& x)
{
  switch (x)
  {
    case PatternInstanceClass::PATTERN_IMAGE_FILL:
      j = "patternImageFill";
      break;
    case PatternInstanceClass::PATTERN_IMAGE_FIT:
      j = "patternImageFit";
      break;
    case PatternInstanceClass::PATTERN_IMAGE_STRETCH:
      j = "patternImageStretch";
      break;
    case PatternInstanceClass::PATTERN_IMAGE_TILE:
      j = "patternImageTile";
      break;
    case PatternInstanceClass::PATTERN_LAYER:
      j = "patternLayer";
      break;
    default:
      break;
  }
}

inline void from_json(const json& j, PatternClass& x)
{
  if (j == "pattern")
    x = PatternClass::PATTERN;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const PatternClass& x)
{
  switch (x)
  {
    case PatternClass::PATTERN:
      j = "pattern";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, FillClass& x)
{
  if (j == "fill")
    x = FillClass::FILL;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const FillClass& x)
{
  switch (x)
  {
    case FillClass::FILL:
      j = "fill";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, FontVariationClass& x)
{
  if (j == "fontVariation")
    x = FontVariationClass::FONT_VARIATION;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const FontVariationClass& x)
{
  switch (x)
  {
    case FontVariationClass::FONT_VARIATION:
      j = "fontVariation";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, FontAttrClass& x)
{
  if (j == "fontAttr")
    x = FontAttrClass::FONT_ATTR;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const FontAttrClass& x)
{
  switch (x)
  {
    case FontAttrClass::FONT_ATTR:
      j = "fontAttr";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, TextParagraphClass& x)
{
  if (j == "textParagraph")
    x = TextParagraphClass::TEXT_PARAGRAPH;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const TextParagraphClass& x)
{
  switch (x)
  {
    case TextParagraphClass::TEXT_PARAGRAPH:
      j = "textParagraph";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, OverrideValueClass& x)
{
  if (j == "overrideValue")
    x = OverrideValueClass::OVERRIDE_VALUE;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const OverrideValueClass& x)
{
  switch (x)
  {
    case OverrideValueClass::OVERRIDE_VALUE:
      j = "overrideValue";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, PointClass& x)
{
  if (j == "pointAttr")
    x = PointClass::POINT_ATTR;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const PointClass& x)
{
  switch (x)
  {
    case PointClass::POINT_ATTR:
      j = "pointAttr";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, RegionClass& x)
{
  if (j == "region")
    x = RegionClass::REGION;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const RegionClass& x)
{
  switch (x)
  {
    case RegionClass::REGION:
      j = "region";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, SegmentClass& x)
{
  if (j == "segment")
    x = SegmentClass::SEGMENT;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const SegmentClass& x)
{
  switch (x)
  {
    case SegmentClass::SEGMENT:
      j = "segment";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, BlurClass& x)
{
  if (j == "blur")
    x = BlurClass::BLUR;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const BlurClass& x)
{
  switch (x)
  {
    case BlurClass::BLUR:
      j = "blur";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, ShadowClass& x)
{
  if (j == "shadow")
    x = ShadowClass::SHADOW;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ShadowClass& x)
{
  switch (x)
  {
    case ShadowClass::SHADOW:
      j = "shadow";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, StyleClass& x)
{
  if (j == "style")
    x = StyleClass::STYLE;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const StyleClass& x)
{
  switch (x)
  {
    case StyleClass::STYLE:
      j = "style";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, TextLineTypeClass& x)
{
  if (j == "textLineType")
    x = TextLineTypeClass::TEXT_LINE_TYPE;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const TextLineTypeClass& x)
{
  switch (x)
  {
    case TextLineTypeClass::TEXT_LINE_TYPE:
      j = "textLineType";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, TextOnPathClass& x)
{
  if (j == "textOnPath")
    x = TextOnPathClass::TEXT_ON_PATH;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const TextOnPathClass& x)
{
  switch (x)
  {
    case TextOnPathClass::TEXT_ON_PATH:
      j = "textOnPath";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, VariableAssignmentClass& x)
{
  if (j == "variableAssign")
    x = VariableAssignmentClass::VARIABLE_ASSIGN;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const VariableAssignmentClass& x)
{
  switch (x)
  {
    case VariableAssignmentClass::VARIABLE_ASSIGN:
      j = "variableAssign";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, VariableDefClass& x)
{
  if (j == "variableDef")
    x = VariableDefClass::VARIABLE_DEF;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const VariableDefClass& x)
{
  switch (x)
  {
    case VariableDefClass::VARIABLE_DEF:
      j = "variableDef";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, VariableRefClass& x)
{
  if (j == "variableRef")
    x = VariableRefClass::VARIABLE_REF;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const VariableRefClass& x)
{
  switch (x)
  {
    case VariableRefClass::VARIABLE_REF:
      j = "variableRef";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, VertexClass& x)
{
  if (j == "vertex")
    x = VertexClass::VERTEX;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const VertexClass& x)
{
  switch (x)
  {
    case VertexClass::VERTEX:
      j = "vertex";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, SubshapeClass& x)
{
  if (j == "subshape")
    x = SubshapeClass::SUBSHAPE;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const SubshapeClass& x)
{
  switch (x)
  {
    case SubshapeClass::SUBSHAPE:
      j = "subshape";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, ShapeClass& x)
{
  if (j == "shape")
    x = ShapeClass::SHAPE;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ShapeClass& x)
{
  switch (x)
  {
    case ShapeClass::SHAPE:
      j = "shape";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, ObjectClass& x)
{
  if (j == "frame")
    x = ObjectClass::FRAME;
  else if (j == "group")
    x = ObjectClass::GROUP;
  else if (j == "image")
    x = ObjectClass::IMAGE;
  else if (j == "path")
    x = ObjectClass::PATH;
  else if (j == "symbolInstance")
    x = ObjectClass::SYMBOL_INSTANCE;
  else if (j == "symbolMaster")
    x = ObjectClass::SYMBOL_MASTER;
  else if (j == "text")
    x = ObjectClass::TEXT;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ObjectClass& x)
{
  switch (x)
  {
    case ObjectClass::FRAME:
      j = "frame";
      break;
    case ObjectClass::GROUP:
      j = "group";
      break;
    case ObjectClass::IMAGE:
      j = "image";
      break;
    case ObjectClass::PATH:
      j = "path";
      break;
    case ObjectClass::SYMBOL_INSTANCE:
      j = "symbolInstance";
      break;
    case ObjectClass::SYMBOL_MASTER:
      j = "symbolMaster";
      break;
    case ObjectClass::TEXT:
      j = "text";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, PatternLayerDefClass& x)
{
  if (j == "patternLayerDef")
    x = PatternLayerDefClass::PATTERN_LAYER_DEF;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const PatternLayerDefClass& x)
{
  switch (x)
  {
    case PatternLayerDefClass::PATTERN_LAYER_DEF:
      j = "patternLayerDef";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, ReferenceClass& x)
{
  if (j == "referencedStyle")
    x = ReferenceClass::REFERENCED_STYLE;
  else if (j == "symbolMaster")
    x = ReferenceClass::SYMBOL_MASTER;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ReferenceClass& x)
{
  switch (x)
  {
    case ReferenceClass::REFERENCED_STYLE:
      j = "referencedStyle";
      break;
    case ReferenceClass::SYMBOL_MASTER:
      j = "symbolMaster";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}
} // namespace Model
} // namespace VGG
namespace nlohmann
{
inline void adl_serializer<std::variant<std::vector<double>, double>>::from_json(
  const json&                                j,
  std::variant<std::vector<double>, double>& x)
{
  if (j.is_number())
    x = j.get<double>();
  else if (j.is_array())
    x = j.get<std::vector<double>>();
  else
    throw std::runtime_error("Could not deserialise!");
}

inline void adl_serializer<std::variant<std::vector<double>, double>>::to_json(
  json&                                            j,
  const std::variant<std::vector<double>, double>& x)
{
  switch (x.index())
  {
    case 0:
      j = std::get<std::vector<double>>(x);
      break;
    case 1:
      j = std::get<double>(x);
      break;
    default:
      throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void adl_serializer<VGG::Model::ContainerChildType>::from_json(
  const json&                     j,
  VGG::Model::ContainerChildType& x)
{
  using namespace VGG::Model;

  if (!j.is_object())
  {
    throw std::runtime_error("Could not deserialise!");
  }

  auto classType = j.at("class").get<ObjectClass>();
  switch (classType)
  {
    case ObjectClass::FRAME:
      x = j.get<Frame>();
      break;
    case ObjectClass::GROUP:
      x = j.get<Group>();
      break;
    case ObjectClass::IMAGE:
      x = j.get<Image>();
      break;
    case ObjectClass::PATH:
      x = j.get<Path>();
      break;
    case ObjectClass::SYMBOL_INSTANCE:
      x = j.get<SymbolInstance>();
      break;
    case ObjectClass::SYMBOL_MASTER:
      x = j.get<SymbolMaster>();
      break;
    case ObjectClass::TEXT:
      x = j.get<Text>();
      break;
    default:
      throw std::runtime_error("Could not deserialise, invalid node type!");
      break;
  }
}

inline void adl_serializer<VGG::Model::ContainerChildType>::to_json(
  json&                                 j,
  const VGG::Model::ContainerChildType& x)
{
  using namespace VGG::Model;

  if (auto p = std::get_if<Frame>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Group>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Image>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Path>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolInstance>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolMaster>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Text>(&x))
  {
    j = *p;
  }
  else
  {
    throw std::runtime_error("Could not serialise, invalid node!");
  }
}

inline void adl_serializer<VGG::Model::SubGeometryType>::from_json(
  const json&                  j,
  VGG::Model::SubGeometryType& x)
{
  using namespace VGG::Model;

  if (!j.is_object())
  {
    throw std::runtime_error("Could not deserialise!");
  }

  auto classType = j.at("class").get<SubGeometryClass>();
  switch (classType)
  {
    case SubGeometryClass::CONTOUR:
      x = j.get<Contour>();
      break;
    case SubGeometryClass::ELLIPSE:
      x = j.get<Ellipse>();
      break;
    case SubGeometryClass::FRAME:
      x = j.get<Frame>();
      break;
    case SubGeometryClass::GROUP:
      x = j.get<Group>();
      break;
    case SubGeometryClass::IMAGE:
      x = j.get<Image>();
      break;
    case SubGeometryClass::PATH:
      x = j.get<Path>();
      break;
    case SubGeometryClass::POLYGON:
      x = j.get<Polygon>();
      break;
    case SubGeometryClass::RECTANGLE:
      x = j.get<Rectangle>();
      break;
    case SubGeometryClass::STAR:
      x = j.get<Star>();
      break;
    case SubGeometryClass::SYMBOL_INSTANCE:
      x = j.get<SymbolInstance>();
      break;
    case SubGeometryClass::SYMBOL_MASTER:
      x = j.get<SymbolMaster>();
      break;
    case SubGeometryClass::TEXT:
      x = j.get<Text>();
      break;
    case SubGeometryClass::VECTOR_NETWORK:
      x = j.get<VectorNetwork>();
      break;
  }
}

inline void adl_serializer<VGG::Model::SubGeometryType>::to_json(
  json&                              j,
  const VGG::Model::SubGeometryType& x)
{
  using namespace VGG::Model;

  if (auto p = std::get_if<Contour>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Ellipse>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Frame>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Group>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Image>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Path>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Polygon>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Rectangle>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Star>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolInstance>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolMaster>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Text>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<VectorNetwork>(&x))
  {
    j = *p;
  }
  else
  {
    throw std::runtime_error("Could not serialise, invalid node!");
  }
}

inline void adl_serializer<VGG::Model::ReferenceType>::from_json(
  const json&                j,
  VGG::Model::ReferenceType& x)
{
  using namespace VGG::Model;

  if (!j.is_object())
  {
    throw std::runtime_error("Could not deserialise!");
  }

  auto classType = j.at("class").get<ReferenceClass>();
  switch (classType)
  {
    case ReferenceClass::REFERENCED_STYLE:
      x = j.get<ReferencedStyle>();
      break;
    case ReferenceClass::SYMBOL_MASTER:
      x = j.get<SymbolMaster>();
      break;
    default:
      throw std::runtime_error("Could not deserialise, invalid node type!");
      break;
  }
}

inline void adl_serializer<VGG::Model::ReferenceType>::to_json(
  json&                            j,
  const VGG::Model::ReferenceType& x)
{
  using namespace VGG::Model;

  if (auto p = std::get_if<ReferencedStyle>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolMaster>(&x))
  {
    j = *p;
  }
  else
  {
    throw std::runtime_error("Could not serialise, invalid node!");
  }
}

inline void adl_serializer<VGG::Model::PatternInstanceType>::from_json(
  const json&                      j,
  VGG::Model::PatternInstanceType& x)
{
  using namespace VGG::Model;

  if (!j.is_object())
  {
    throw std::runtime_error("Could not deserialise!");
  }

  auto classType = j.at("class").get<PatternInstanceClass>();
  switch (classType)
  {
    case PatternInstanceClass::PATTERN_IMAGE_FILL:
      x = j.get<PatternImageFill>();
      break;
    case PatternInstanceClass::PATTERN_IMAGE_FIT:
      x = j.get<PatternImageFit>();
      break;
    case PatternInstanceClass::PATTERN_IMAGE_STRETCH:
      x = j.get<PatternImageStrech>();
      break;
    case PatternInstanceClass::PATTERN_IMAGE_TILE:
      x = j.get<PatternImageTile>();
      break;
    case PatternInstanceClass::PATTERN_LAYER:
      x = j.get<PatternLayerInstance>();
      break;
    default:
      break;
  }
}

inline void adl_serializer<VGG::Model::PatternInstanceType>::to_json(
  json&                                  j,
  const VGG::Model::PatternInstanceType& x)
{
  using namespace VGG::Model;

  if (auto p = std::get_if<PatternImageFill>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<PatternImageFit>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<PatternImageStrech>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<PatternImageTile>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<PatternLayerInstance>(&x))
  {
    j = *p;
  }
}
} // namespace nlohmann
