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

struct Frame;
struct Group;
struct Image;
struct Path;
struct SymbolInstance;
struct SymbolMaster;
struct Text;
using ContainerChildType =
  std::variant<std::monostate, Frame, Group, Image, Path, SymbolInstance, SymbolMaster, Text>;

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
  int64_t          alphaType;
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
  double               alpha;
  double               blue;
  BackgroundColorClass colorClass;
  double               green;
  double               red;
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
  int64_t              blendMode;
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
  int64_t              transparencyKnockoutGroup;
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
  int64_t             flag;
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
  int64_t             flag;
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
  PATTERN_LAYER
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
  int64_t                 fillType;
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
  int64_t                 lineCapStyle;
  /**
   * The shape style at the corner of two border lines.
   */
  int64_t                 lineJoinStyle;
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
  int64_t                 position;
  /**
   * This is used only when `borderWeightsIndependent` is set to true, in order to specify the
   * border weight at the right of the rectangle. Default value is `0`.
   */
  std::optional<double>   rightWeight;
  /**
   * The type of the border style. The dashed style is further specified in `dashedOffset` and
   * `dashedPattern`.
   */
  int64_t                 style;
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
  int64_t                 fillType;
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
  std::optional<int64_t>                    fillUseType;
  /**
   * The type of small caps.
   * **Note**:
   * `textCase` and `fontVariantCaps` are mutually exclusive. If either item is nonzero, you
   * can ignore the value of the other, as they will not be nonzero at the same time.
   */
  std::optional<int64_t>                    fontVariantCaps;
  /**
   * The position of the text characters.
   */
  std::optional<int64_t>                    fontVariantPosition;
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
  std::optional<int64_t>                    length;
  /**
   * The unit of `letterSpacingValue` value.
   */
  std::optional<int64_t>                    letterSpacingUnit;
  /**
   * Text character spacing value (can be negative).
   * Must be used together with `letterSpacingUnit`.
   * Default value is `0`.
   */
  std::optional<double>                     letterSpacingValue;
  /**
   * The unit of `lineSpacingValue` value.
   */
  std::optional<int64_t>                    lineSpacingUnit;
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
  std::optional<int64_t>                    textCase;
  /**
   * The properties of the text paragraph, which are consistent across the same text paragraph.
   */
  std::optional<TextParagraph>              textParagraph;
  /**
   * The underline type of the text.
   */
  std::optional<int64_t>                    underline;
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
  PointClass                         pointAttrClass;
  /**
   * Corner shapes of the point.
   */
  std::optional<int64_t>             cornerStyle;
  /**
   * When the current point is the starting point of the Bezier curve, this attribute
   * represents its control point.
   */
  std::optional<std::vector<double>> curveFrom;
  /**
   * When the current point is the end point of the Bezier curve, this attribute represents
   * its control point.
   */
  std::optional<std::vector<double>> curveTo;
  /**
   * The shape type of the point. This property only applies to endpoints.
   */
  std::optional<int64_t>             markType;
  /**
   * The coordinates of the point before the matrix transformation.
   */
  std::vector<double>                point;
  /**
   * Corner radius at the point.
   */
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
  int64_t                                  windingRule;
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
  int64_t                            end;
  /**
   * The starting point of the segment. The number is the index of the `vertices` list in
   * `Vector NetWork`, starting from `0`.
   */
  int64_t                            start;
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
  int64_t               type;
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
  int64_t           level;
  int64_t           styleType;
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
  int64_t          varType;
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
  VertexClass            vertexClass;
  /**
   * The shape type of the vertex. This property only applies to endpoints.
   */
  std::optional<int64_t> markType;
  /**
   * The coordinates of the point before the matrix transformation.
   */
  std::vector<double>    point;
  /**
   * Corner radius at the vertex.
   */
  std::optional<double>  radius;
};

enum class SubshapeClass : int
{
  SUBSHAPE
};

enum class ShapeClass : int
{
  SHAPE
};

enum class ChildObjectClass : int
{
  FRAME,
  GROUP,
  IMAGE,
  PATH,
  SYMBOL_INSTANCE,
  SYMBOL_MASTER,
  TEXT
};

struct Contour;
struct VectorNetwork;
struct Ellipse;
struct Polygon;
struct Rectangle;
struct Star;
using SubGeometryType = std::variant<
  std::monostate,
  Contour,
  Ellipse,
  Frame,
  Group,
  Image,
  Path,
  Polygon,
  Rectangle,
  Star,
  SymbolInstance,
  SymbolMaster,
  Text,
  VectorNetwork>;

struct Subshape;

struct Shape;

struct ReferencedStyle;
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
  /**
   * Boolean operations that combine the current subshape with the previous subshape in the
   * array.
   */
  int64_t                          booleanOperation;
  SubshapeClass                    subshapeClass;
  /**
   * Describes the detailed shape of the subshape through a contour with a list of points, a
   * specified geometry, or another object.
   * When the `frame`, `symbol-instance`, and `symbol-master` are in `subGeometry`, only the
   * `childObjects` will be used, and the bounding box will be ignored.
   * Any mask nested within this object will be invalidated.
   */
  std::shared_ptr<SubGeometryType> subGeometry;
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
  int64_t               windingRule;
};

/**
 * Path represents a vector geometry which is formed by individual subpaths combined
 * together via boolean operations.
 *
 * Details of an image.
 *
 * Details of a text.
 *
 * A group combines a list of objects, but it is not considered an object itself.
 * In contrast to a frame, a frame is an object that can be rendered and a container that
 * holds objects.
 * For example:
 * The group's fills affect all of its children.
 * The frame's fills affect itself.
 *
 * A frame is an object that can be rendered and a container that holds objects.
 * In contrast to a group, a group is not an object but rather a collection of objects.
 * For example:
 * The frame's fills affect itself.
 * The group's fills affect all of its children.
 *
 * Symbol instance object is an instance of a symbol master which can be overriden.
 *
 * A symbol master is a reusable object that contains a group of objects.
 */
struct Path
{
  /**
   * A list of alpha masks applied to the object.
   */
  std::vector<AlphaMask>                         alphaMaskBy;
  /**
   * The bounds of the object before undergoing matrix transformations.
   */
  Rect                                           bounds;
  ChildObjectClass                               pathClass;
  /**
   * The opacity and blending-related configurations of the object.
   */
  GraphicsContextSettings                        contextSettings;
  /**
   * Smoothness of rounded corners. Range: `[0, 1]`.
   * `0` is the default value, indicating no smoothing for rounded corners.
   */
  std::optional<double>                          cornerSmoothing;
  /**
   * Horizontal constraints for the object.
   * Default value is `1`.
   */
  std::optional<int64_t>                         horizontalConstraint;
  /**
   * ID of the object, globally unique.
   */
  std::string                                    id;
  /**
   * If `true`, the object will be unable to be edited.
   */
  bool                                           isLocked;
  /**
   * `False`: When resizing occurs, the object scales according to the `horizontalConstraint`
   * and `verticalConstraint`. (Default value.)
   * `True`: When resizing occurs, the object itself maintains its angle, and the center
   * position is scaled. The scaling occurs along both the length and width directions.
   */
  std::optional<bool>                            keepShapeWhenResize;
  /**
   * How the mask object is displayed.
   */
  std::optional<int64_t>                         maskShowType;
  /**
   * The mask type of the object.
   */
  int64_t                                        maskType;
  /**
   * Matrix used for translating, rotating, and scaling the object.
   */
  std::vector<double>                            matrix;
  /**
   * Name of the object, for user identification, encoded in UTF-8.
   */
  std::optional<std::string>                     name;
  /**
   * A list of outline masks applied to the object, clipped by the intersection of their
   * outlines.
   * The items in the list are object IDs of the masks.
   */
  std::vector<std::string>                       outlineMaskBy;
  /**
   * How to display the child element of the object when it overflows its container.
   */
  int64_t                                        overflow;
  /**
   * Used to be associated with the object by symbol instances for overriding its attributes.
   * Check the `objectId` in the `OverrideValue` for details.
   * If `overrideKey` exists, find a symbol master through upward traversal (which could be
   * the object itself); `overrideKey` is unique within the symbol master.
   */
  std::optional<std::string>                     overrideKey;
  /**
   * How child objects behave when the object is resized.
   */
  std::optional<int64_t>                         resizesContent;
  /**
   * The shape of the path, describing the details of the path.
   */
  std::shared_ptr<Shape>                         shape;
  /**
   * The borders, fills, and other styles of the object.
   */
  Style                                          style;
  /**
   * How the `style` of the object affects the region participating in a Boolean operation
   * with another object.
   */
  std::optional<int64_t>                         styleEffectBoolean;
  /**
   * How the `style` and `visible` of the mask object affect the area of the mask.
   */
  int64_t                                        styleEffectMaskArea;
  /**
   * An enclosing rectangle for the object. This property stores information about the object
   * after the matrix transformation.
   */
  std::optional<Rect>                            transformedBounds;
  /**
   * A list of variables that can be used by children.
   */
  std::optional<std::vector<VariableDefine>>     variableDefs;
  /**
   * A list of referenced variables.
   */
  std::optional<std::vector<VariableRefer>>      variableRefs;
  /**
   * Vertical constraints for the object.
   * Default value is `1`.
   */
  std::optional<int64_t>                         verticalConstraint;
  /**
   * If `false`, the object will be invisible.
   */
  bool                                           visible;
  /**
   * If the value is `true`, the image content is not displayed, and only the `fill` effect is
   * shown.
   * Otherwise, both the image content and the `fill` effect take effect simultaneously.
   * Default value is `false`.
   */
  std::optional<bool>                            fillReplacesImage;
  /**
   * The path (UTF-8) of the image file.
   */
  std::optional<std::string>                     imageFileName;
  /**
   * Adjust the colors of the image.
   */
  std::optional<ImageFilters>                    imageFilters;
  /**
   * The position of the first character baseline when text is drawn.
   * Horizontal and vertical alignments have been considered. The given coordinates are in the
   * object's own coordinate system.
   * If this property doesn't exist, text is drawn using `bounds`.
   */
  std::optional<std::vector<double>>             anchorPoint;
  /**
   * The text content of the text object.
   * The encoding format is UTF-8.
   */
  std::optional<std::string>                     content;
  /**
   * The default font attributes of the text. If some font attributes are missing in
   * `fontAttr`, take the font attributes here.
   * The value of `length` in this field is meaningless.
   */
  std::optional<TextFontAttributes>              defaultFontAttr;
  /**
   * An ordered list, where each item sequentially describes the font attributes of a text
   * fragment.
   * If some font attributes are missing in an item, take the font attributes from
   * `defaultFontAttr`.
   */
  std::optional<std::vector<TextFontAttributes>> fontAttr;
  /**
   * The mode of the text frame size.
   */
  std::optional<int64_t>                         frameMode;
  /**
   * The type of horizontal alignment for the text. When the number of items is less than the
   * number of rows, the last value is reused.
   */
  std::optional<std::vector<int64_t>>            horizontalAlignment;
  /**
   * A list with a length equals to the number of text lines, where each item describes the
   * index preceding each line of text.
   */
  std::optional<std::vector<TextLineType>>       textLineType;
  /**
   * The text is arranged along the path.
   */
  std::optional<TextOnPath>                      textOnPath;
  /**
   * The maximum height that can be displayed inside the text frame, with any text exceeding
   * this height being truncated.
   */
  std::optional<double>                          truncatedHeight;
  /**
   * The type of vertical alignment for the text.
   */
  std::optional<int64_t>                         verticalAlignment;
  /**
   * If true, trim the text portion that extends beyond the text frame starting from the
   * baseline, based on `verticalAlignment`
   * Default value is `false`.
   */
  std::optional<bool>                            verticalTrim;
  /**
   * A list of all child objects.
   * **Note:** The child object that appears later in the list will be displayed above the one
   * that appears first.
   */
  std::optional<std::vector<Path>>               childObjects;
  /**
   * When the group itself is a mask and the group contains a mask, this value affects the
   * valid area of the group as a mask.
   * `True`: The mask region of the group is the result after the inner mask.
   * `False`: A mask inside a group does not affect the region of the group's mask (default
   * value).
   * If the object does not act as a mask or does not have a mask child inside it, then this
   * property is ignored.
   */
  std::optional<bool>                            groupNestMaskType;
  /**
   * This field is reserved for `vector-network` compatibility and is only true if
   * `vector-network` is converted to a group.
   * Default value is `false`.
   */
  std::optional<bool>                            isVectorNetwork;
  /**
   * The background color of the canvas on which the frame is positioned. Exists only on the
   * top-level frame.
   */
  std::optional<Color>                           backgroundColor;
  /**
   * The radius values correspond to the corners in the following order: left-top, right-top,
   * right-bottom, left-bottom.
   * Default value is `[0, 0, 0, 0]`.
   */
  std::optional<std::vector<double>>             radius;
  /**
   * The object ID of the symbol master.
   */
  std::optional<std::string>                     masterId;
  /**
   * A list of overridden values.
   */
  std::optional<std::vector<OverrideValue>>      overrideValues;
  /**
   * Reassign the value of the variable defined in symbol master.
   */
  std::optional<std::vector<VariableAssign>>     variableAssignments;
};

enum class FrameClass : int
{
  FRAME
};

/**
 * A frame is an object that can be rendered and a container that holds objects.
 * In contrast to a group, a group is not an object but rather a collection of objects.
 * For example:
 * The frame's fills affect itself.
 * The group's fills affect all of its children.
 */
struct Frame
{
  /**
   * A list of alpha masks applied to the object.
   */
  std::vector<AlphaMask>                     alphaMaskBy;
  /**
   * The background color of the canvas on which the frame is positioned. Exists only on the
   * top-level frame.
   */
  std::optional<Color>                       backgroundColor;
  /**
   * The bounds of the object before undergoing matrix transformations.
   */
  Rect                                       bounds;
  /**
   * A list of all child objects.
   * **Note:** The child object that appears later in the list will be displayed above the one
   * that appears first.
   */
  std::vector<ContainerChildType>            childObjects;
  FrameClass                                 frameClass;
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
  std::optional<int64_t>                     horizontalConstraint;
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
  std::optional<int64_t>                     maskShowType;
  /**
   * The mask type of the object.
   */
  int64_t                                    maskType;
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
  int64_t                                    overflow;
  /**
   * Used to be associated with the object by symbol instances for overriding its attributes.
   * Check the `objectId` in the `OverrideValue` for details.
   * If `overrideKey` exists, find a symbol master through upward traversal (which could be
   * the object itself); `overrideKey` is unique within the symbol master.
   */
  std::optional<std::string>                 overrideKey;
  /**
   * The radius values correspond to the corners in the following order: left-top, right-top,
   * right-bottom, left-bottom.
   * Default value is `[0, 0, 0, 0]`.
   */
  std::optional<std::vector<double>>         radius;
  /**
   * How child objects behave when the object is resized.
   */
  std::optional<int64_t>                     resizesContent;
  /**
   * The borders, fills, and other styles of the object.
   */
  Style                                      style;
  /**
   * How the `style` of the object affects the region participating in a Boolean operation
   * with another object.
   */
  std::optional<int64_t>                     styleEffectBoolean;
  /**
   * How the `style` and `visible` of the mask object affect the area of the mask.
   */
  int64_t                                    styleEffectMaskArea;
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
  std::optional<int64_t>                     verticalConstraint;
  /**
   * If `false`, the object will be invisible.
   */
  bool                                       visible;
};

struct Group
{
  std::vector<AlphaMask>                     alphaMaskBy;
  Rect                                       bounds;
  std::vector<ContainerChildType>            childObjects;
  ChildObjectClass                           class_;
  GraphicsContextSettings                    contextSettings;
  std::optional<double>                      cornerSmoothing;
  std::optional<bool>                        groupNestMaskType;
  std::optional<int64_t>                     horizontalConstraint;
  std::string                                id;
  bool                                       isLocked;
  std::optional<bool>                        isVectorNetwork;
  std::optional<bool>                        keepShapeWhenResize;
  std::optional<int64_t>                     maskShowType;
  int64_t                                    maskType;
  std::vector<double>                        matrix;
  std::optional<std::string>                 name;
  std::vector<std::string>                   outlineMaskBy;
  int64_t                                    overflow;
  std::optional<std::string>                 overrideKey;
  std::optional<int64_t>                     resizesContent;
  Style                                      style;
  std::optional<int64_t>                     styleEffectBoolean;
  int64_t                                    styleEffectMaskArea;
  std::optional<Rect>                        transformedBounds;
  std::optional<std::vector<VariableDefine>> variableDefs;
  std::optional<std::vector<VariableRefer>>  variableRefs;
  std::optional<int64_t>                     verticalConstraint;
  bool                                       visible;
};

struct Image
{
  std::vector<AlphaMask>                     alphaMaskBy;
  Rect                                       bounds;
  ChildObjectClass                           class_;
  GraphicsContextSettings                    contextSettings;
  std::optional<double>                      cornerSmoothing;
  std::optional<bool>                        fillReplacesImage;
  std::optional<int64_t>                     horizontalConstraint;
  std::string                                id;
  std::string                                imageFileName;
  std::optional<ImageFilters>                imageFilters;
  std::optional<bool>                        keepShapeWhenResize;
  std::optional<int64_t>                     maskShowType;
  int64_t                                    maskType;
  std::vector<double>                        matrix;
  std::optional<std::string>                 name;
  std::vector<std::string>                   outlineMaskBy;
  int64_t                                    overflow;
  std::optional<std::string>                 overrideKey;
  std::optional<int64_t>                     resizesContent;
  Style                                      style;
  std::optional<int64_t>                     styleEffectBoolean;
  int64_t                                    styleEffectMaskArea;
  std::optional<Rect>                        transformedBounds;
  std::optional<std::vector<VariableDefine>> variableDefs;
  std::optional<std::vector<VariableRefer>>  variableRefs;
  std::optional<int64_t>                     verticalConstraint;
  bool                                       visible;
};

struct SymbolInstance
{
  std::vector<AlphaMask>                     alphaMaskBy;
  Rect                                       bounds;
  ChildObjectClass                           class_;
  GraphicsContextSettings                    contextSettings;
  std::optional<double>                      cornerSmoothing;
  std::optional<int64_t>                     horizontalConstraint;
  std::string                                id;
  bool                                       isLocked;
  std::optional<bool>                        keepShapeWhenResize;
  std::optional<int64_t>                     maskShowType;
  int64_t                                    maskType;
  std::string                                masterId;
  std::vector<double>                        matrix;
  std::optional<std::string>                 name;
  std::vector<std::string>                   outlineMaskBy;
  int64_t                                    overflow;
  std::optional<std::string>                 overrideKey;
  std::vector<OverrideValue>                 overrideValues;
  std::optional<int64_t>                     resizesContent;
  Style                                      style;
  std::optional<int64_t>                     styleEffectBoolean;
  int64_t                                    styleEffectMaskArea;
  std::optional<Rect>                        transformedBounds;
  std::optional<std::vector<VariableAssign>> variableAssignments;
  std::optional<std::vector<VariableDefine>> variableDefs;
  std::optional<std::vector<VariableRefer>>  variableRefs;
  std::optional<int64_t>                     verticalConstraint;
  bool                                       visible;
};

struct SymbolMaster
{
  std::vector<AlphaMask>                     alphaMaskBy;
  Rect                                       bounds;
  std::vector<ContainerChildType>            childObjects;
  ChildObjectClass                           class_;
  GraphicsContextSettings                    contextSettings;
  std::optional<double>                      cornerSmoothing;
  std::optional<int64_t>                     horizontalConstraint;
  std::string                                id;
  bool                                       isLocked;
  std::optional<bool>                        keepShapeWhenResize;
  std::optional<int64_t>                     maskShowType;
  int64_t                                    maskType;
  std::vector<double>                        matrix;
  std::optional<std::string>                 name;
  std::vector<std::string>                   outlineMaskBy;
  int64_t                                    overflow;
  std::optional<std::string>                 overrideKey;
  std::vector<double>                        radius;
  std::optional<int64_t>                     resizesContent;
  Style                                      style;
  std::optional<int64_t>                     styleEffectBoolean;
  int64_t                                    styleEffectMaskArea;
  std::optional<Rect>                        transformedBounds;
  std::optional<std::vector<VariableDefine>> variableDefs;
  std::optional<std::vector<VariableRefer>>  variableRefs;
  std::optional<int64_t>                     verticalConstraint;
  bool                                       visible;
};

struct Text
{
  std::vector<AlphaMask>                     alphaMaskBy;
  std::optional<std::vector<double>>         anchorPoint;
  Rect                                       bounds;
  ChildObjectClass                           class_;
  std::string                                content;
  GraphicsContextSettings                    contextSettings;
  std::optional<double>                      cornerSmoothing;
  std::optional<TextFontAttributes>          defaultFontAttr;
  std::vector<TextFontAttributes>            fontAttr;
  int                                        frameMode;
  std::vector<int>                           horizontalAlignment;
  std::optional<int64_t>                     horizontalConstraint;
  std::string                                id;
  bool                                       isLocked;
  std::optional<bool>                        keepShapeWhenResize;
  std::optional<int64_t>                     maskShowType;
  int64_t                                    maskType;
  std::vector<double>                        matrix;
  std::optional<std::string>                 name;
  std::vector<std::string>                   outlineMaskBy;
  int64_t                                    overflow;
  std::optional<std::string>                 overrideKey;
  std::optional<int64_t>                     resizesContent;
  Style                                      style;
  std::optional<int64_t>                     styleEffectBoolean;
  int64_t                                    styleEffectMaskArea;
  std::optional<std::vector<TextLineType>>   textLineType;
  std::optional<TextOnPath>                  textOnPath;
  std::optional<Rect>                        transformedBounds;
  std::optional<double>                      truncatedHeight;
  std::optional<std::vector<VariableDefine>> variableDefs;
  std::optional<std::vector<VariableRefer>>  variableRefs;
  int                                        verticalAlignment;
  std::optional<int64_t>                     verticalConstraint;
  std::optional<bool>                        verticalTrim;
  bool                                       visible;
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
  std::optional<int64_t>                     horizontalConstraint;
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
  std::optional<int64_t>                     maskShowType;
  /**
   * The mask type of the object.
   */
  int64_t                                    maskType;
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
  int64_t                                    overflow;
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
  std::optional<int64_t>                     resizesContent;
  /**
   * The borders, fills, and other styles of the object.
   */
  Style                                      style;
  /**
   * How the `style` of the object affects the region participating in a Boolean operation
   * with another object.
   */
  std::optional<int64_t>                     styleEffectBoolean;
  /**
   * How the `style` and `visible` of the mask object affect the area of the mask.
   */
  int64_t                                    styleEffectMaskArea;
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
  std::optional<int64_t>                     verticalConstraint;
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

enum class Version : int
{
  THE_1016
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
  int64_t                                     fileType;
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
  Version                                     version;
};
} // namespace Model
} // namespace VGG

namespace VGG
{
namespace Model
{
void from_json(const json& j, AlphaMask& x);
void to_json(json& j, const AlphaMask& x);

void from_json(const json& j, Color& x);
void to_json(json& j, const Color& x);

void from_json(const json& j, Rect& x);
void to_json(json& j, const Rect& x);

void from_json(const json& j, GraphicsContextSettings& x);
void to_json(json& j, const GraphicsContextSettings& x);

void from_json(const json& j, GradientBasicGeometry& x);
void to_json(json& j, const GradientBasicGeometry& x);

void from_json(const json& j, GradientHilight& x);
void to_json(json& j, const GradientHilight& x);

void from_json(const json& j, PerpendicularMatrix& x);
void to_json(json& j, const PerpendicularMatrix& x);

void from_json(const json& j, GradientStop& x);
void to_json(json& j, const GradientStop& x);

void from_json(const json& j, GradientInstance& x);
void to_json(json& j, const GradientInstance& x);

void from_json(const json& j, Gradient& x);
void to_json(json& j, const Gradient& x);

void from_json(const json& j, ImageFilters& x);
void to_json(json& j, const ImageFilters& x);

void from_json(const json& j, PatternImageFill& x);
void to_json(json& j, const PatternImageFill& x);
void from_json(const json& j, PatternImageStrech& x);
void to_json(json& j, const PatternImageStrech& x);
void from_json(const json& j, PatternImageFit& x);
void to_json(json& j, const PatternImageFit& x);
void from_json(const json& j, PatternImageTile& x);
void to_json(json& j, const PatternImageTile& x);
void from_json(const json& j, PatternLayerInstance& x);
void to_json(json& j, const PatternLayerInstance& x);

void from_json(const json& j, Pattern& x);
void to_json(json& j, const Pattern& x);

void from_json(const json& j, Border& x);
void to_json(json& j, const Border& x);

void from_json(const json& j, Fill& x);
void to_json(json& j, const Fill& x);

void from_json(const json& j, FontVariation& x);
void to_json(json& j, const FontVariation& x);

void from_json(const json& j, TextParagraph& x);
void to_json(json& j, const TextParagraph& x);

void from_json(const json& j, TextFontAttributes& x);
void to_json(json& j, const TextFontAttributes& x);

void from_json(const json& j, OverrideValue& x);
void to_json(json& j, const OverrideValue& x);

void from_json(const json& j, PointAttr& x);
void to_json(json& j, const PointAttr& x);

void from_json(const json& j, Region& x);
void to_json(json& j, const Region& x);

void from_json(const json& j, Segment& x);
void to_json(json& j, const Segment& x);

void from_json(const json& j, Blur& x);
void to_json(json& j, const Blur& x);

void from_json(const json& j, Shadow& x);
void to_json(json& j, const Shadow& x);

void from_json(const json& j, Style& x);
void to_json(json& j, const Style& x);

void from_json(const json& j, TextLineType& x);
void to_json(json& j, const TextLineType& x);

void from_json(const json& j, TextOnPath& x);
void to_json(json& j, const TextOnPath& x);

void from_json(const json& j, VariableAssign& x);
void to_json(json& j, const VariableAssign& x);

void from_json(const json& j, VariableDefine& x);
void to_json(json& j, const VariableDefine& x);

void from_json(const json& j, VariableRefer& x);
void to_json(json& j, const VariableRefer& x);

void from_json(const json& j, Vertex& x);
void to_json(json& j, const Vertex& x);

void from_json(const json& j, Contour& x);
void to_json(json& j, const Contour& x);
void from_json(const json& j, VectorNetwork& x);
void to_json(json& j, const VectorNetwork& x);
void from_json(const json& j, Ellipse& x);
void to_json(json& j, const Ellipse& x);
void from_json(const json& j, Polygon& x);
void to_json(json& j, const Polygon& x);
void from_json(const json& j, Rectangle& x);
void to_json(json& j, const Rectangle& x);
void from_json(const json& j, Star& x);
void to_json(json& j, const Star& x);

void from_json(const json& j, Subshape& x);
void to_json(json& j, const Subshape& x);

void from_json(const json& j, Shape& x);
void to_json(json& j, const Shape& x);

void from_json(const json& j, Path& x);
void to_json(json& j, const Path& x);

void from_json(const json& j, Frame& x);
void to_json(json& j, const Frame& x);

void from_json(const json& j, Group& x);
void to_json(json& j, const Group& x);

void from_json(const json& j, Image& x);
void to_json(json& j, const Image& x);

void from_json(const json& j, SymbolInstance& x);
void to_json(json& j, const SymbolInstance& x);

void from_json(const json& j, SymbolMaster& x);
void to_json(json& j, const SymbolMaster& x);

void from_json(const json& j, Text& x);
void to_json(json& j, const Text& x);

void from_json(const json& j, PatternLayerDef& x);
void to_json(json& j, const PatternLayerDef& x);

void from_json(const json& j, ReferencedStyle& x);
void to_json(json& j, const ReferencedStyle& x);

void from_json(const json& j, DesignModel& x);
void to_json(json& j, const DesignModel& x);

void from_json(const json& j, AlphaMaskByClass& x);
void to_json(json& j, const AlphaMaskByClass& x);

void from_json(const json& j, BackgroundColorClass& x);
void to_json(json& j, const BackgroundColorClass& x);

void from_json(const json& j, BoundsClass& x);
void to_json(json& j, const BoundsClass& x);

void from_json(const json& j, ContextSettingsClass& x);
void to_json(json& j, const ContextSettingsClass& x);

void from_json(const json& j, SubGeometryClass& x);
void to_json(json& j, const SubGeometryClass& x);

void from_json(const json& j, BorderClass& x);
void to_json(json& j, const BorderClass& x);

void from_json(const json& j, GradientClass& x);
void to_json(json& j, const GradientClass& x);

void from_json(const json& j, GeometryClass& x);
void to_json(json& j, const GeometryClass& x);

void from_json(const json& j, GradientInstanceClass& x);
void to_json(json& j, const GradientInstanceClass& x);

void from_json(const json& j, HilightClass& x);
void to_json(json& j, const HilightClass& x);

void from_json(const json& j, StopClass& x);
void to_json(json& j, const StopClass& x);

void from_json(const json& j, ImageFiltersClass& x);
void to_json(json& j, const ImageFiltersClass& x);

void from_json(const json& j, PatternInstanceClass& x);
void to_json(json& j, const PatternInstanceClass& x);

void from_json(const json& j, PatternClass& x);
void to_json(json& j, const PatternClass& x);

void from_json(const json& j, FillClass& x);
void to_json(json& j, const FillClass& x);

void from_json(const json& j, FontVariationClass& x);
void to_json(json& j, const FontVariationClass& x);

void from_json(const json& j, FontAttrClass& x);
void to_json(json& j, const FontAttrClass& x);

void from_json(const json& j, TextParagraphClass& x);
void to_json(json& j, const TextParagraphClass& x);

void from_json(const json& j, OverrideValueClass& x);
void to_json(json& j, const OverrideValueClass& x);

void from_json(const json& j, PointClass& x);
void to_json(json& j, const PointClass& x);

void from_json(const json& j, RegionClass& x);
void to_json(json& j, const RegionClass& x);

void from_json(const json& j, SegmentClass& x);
void to_json(json& j, const SegmentClass& x);

void from_json(const json& j, BlurClass& x);
void to_json(json& j, const BlurClass& x);

void from_json(const json& j, ShadowClass& x);
void to_json(json& j, const ShadowClass& x);

void from_json(const json& j, StyleClass& x);
void to_json(json& j, const StyleClass& x);

void from_json(const json& j, TextLineTypeClass& x);
void to_json(json& j, const TextLineTypeClass& x);

void from_json(const json& j, TextOnPathClass& x);
void to_json(json& j, const TextOnPathClass& x);

void from_json(const json& j, VariableAssignmentClass& x);
void to_json(json& j, const VariableAssignmentClass& x);

void from_json(const json& j, VariableDefClass& x);
void to_json(json& j, const VariableDefClass& x);

void from_json(const json& j, VariableRefClass& x);
void to_json(json& j, const VariableRefClass& x);

void from_json(const json& j, VertexClass& x);
void to_json(json& j, const VertexClass& x);

void from_json(const json& j, SubshapeClass& x);
void to_json(json& j, const SubshapeClass& x);

void from_json(const json& j, ShapeClass& x);
void to_json(json& j, const ShapeClass& x);

void from_json(const json& j, ChildObjectClass& x);
void to_json(json& j, const ChildObjectClass& x);

void from_json(const json& j, FrameClass& x);
void to_json(json& j, const FrameClass& x);

void from_json(const json& j, PatternLayerDefClass& x);
void to_json(json& j, const PatternLayerDefClass& x);

void from_json(const json& j, ReferenceClass& x);
void to_json(json& j, const ReferenceClass& x);

void from_json(const json& j, Version& x);
void to_json(json& j, const Version& x);
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
  x.alphaType = j.at("alphaType").get<int64_t>();
  x.alphaMaskClass = j.at("class").get<AlphaMaskByClass>();
  x.crop = j.at("crop").get<bool>();
  x.id = j.at("id").get<std::string>();
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
  x.alpha = j.at("alpha").get<double>();
  x.blue = j.at("blue").get<double>();
  x.colorClass = j.at("class").get<BackgroundColorClass>();
  x.green = j.at("green").get<double>();
  x.red = j.at("red").get<double>();
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
  x.rectClass = j.at("class").get<BoundsClass>();
  x.constrainProportions = j.at("constrainProportions").get<bool>();
  x.height = j.at("height").get<double>();
  x.width = j.at("width").get<double>();
  x.x = j.at("x").get<double>();
  x.y = j.at("y").get<double>();
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
  x.blendMode = j.at("blendMode").get<int64_t>();
  x.graphicsContextSettingsClass = j.at("class").get<ContextSettingsClass>();
  x.isolateBlending = j.at("isolateBlending").get<bool>();
  x.opacity = j.at("opacity").get<double>();
  x.transparencyKnockoutGroup = j.at("transparencyKnockoutGroup").get<int64_t>();
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
  x.angle = j.at("angle").get<double>();
  x.gradientBasicGeometryClass = j.at("class").get<GeometryClass>();
  x.flag = j.at("flag").get<int64_t>();
  x.length = j.at("length").get<double>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.widthRatio = j.at("widthRatio").get<double>();
  x.xOrigin = j.at("xOrigin").get<double>();
  x.yOrigin = j.at("yOrigin").get<double>();
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
  x.angle = j.at("angle").get<double>();
  x.gradientHilightClass = j.at("class").get<HilightClass>();
  x.length = j.at("length").get<double>();
  x.xHilight = j.at("xHilight").get<double>();
  x.yHilight = j.at("yHilight").get<double>();
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
  x.flag = j.at("flag").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
}

inline void to_json(json& j, const PerpendicularMatrix& x)
{
  j = json::object();
  j["flag"] = x.flag;
  j["matrix"] = x.matrix;
}

inline void from_json(const json& j, GradientStop& x)
{
  x.gradientStopClass = j.at("class").get<StopClass>();
  x.color = j.at("color").get<Color>();
  x.midPoint = j.at("midPoint").get<double>();
  x.position = j.at("position").get<double>();
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
  x.gradientClass = j.at("class").get<GradientInstanceClass>();
  x.from = j.at("from").get<std::vector<double>>();
  x.to = j.at("to").get<std::vector<double>>();
  x.stops = j.at("stops").get<std::vector<GradientStop>>();
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
  x.gradientClass = j.at("class").get<GradientClass>();
  x.instance = j.at("instance").get<GradientInstance>();
}

inline void to_json(json& j, const Gradient& x)
{
  j = json::object();
  j["class"] = x.gradientClass;
  j["instance"] = x.instance;
}

inline void from_json(const json& j, ImageFilters& x)
{
  x.imageFiltersClass = j.at("class").get<ImageFiltersClass>();
  x.contrast = get_stack_optional<double>(j, "contrast");
  x.exposure = get_stack_optional<double>(j, "exposure");
  x.highlights = get_stack_optional<double>(j, "highlights");
  x.hue = get_stack_optional<double>(j, "hue");
  x.isEnabled = j.at("isEnabled").get<bool>();
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
  x.class_ = j.at("class").get<PatternInstanceClass>();
  x.imageFileName = j.at("imageFileName").get<std::string>();
  x.rotation = j.at("rotation").get<double>();
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

void from_json(const json& j, PatternImageStrech& x)
{
  x.class_ = j.at("class").get<PatternInstanceClass>();
  x.imageFileName = j.at("imageFileName").get<std::string>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}
void to_json(json& j, const PatternImageStrech& x)
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
  x.class_ = j.at("class").get<PatternInstanceClass>();
  x.imageFileName = j.at("imageFileName").get<std::string>();
  x.rotation = j.at("rotation").get<double>();
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

void from_json(const json& j, PatternImageTile& x)
{
  x.class_ = j.at("class").get<PatternInstanceClass>();
  x.scale = j.at("scale").get<double>();
  x.imageFileName = j.at("imageFileName").get<std::string>();
  x.rotation = j.at("rotation").get<double>();
  x.mirror = get_stack_optional<bool>(j, "mirror");
  x.mode = get_stack_optional<int>(j, "mode");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
}
void to_json(json& j, const PatternImageTile& x)
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

void from_json(const json& j, PatternLayerInstance& x)
{
  x.class_ = j.at("class").get<PatternInstanceClass>();
  x.refLayerName = j.at("refLayerName").get<std::string>();
  x.offset = j.at("offset").get<std::vector<double>>();
  x.scale = j.at("scale").get<std::vector<double>>();
  x.angle = j.at("angle").get<double>();
  x.reflection = j.at("reflection").get<bool>();
  x.r = j.at("r").get<double>();
  x.shear = j.at("shear").get<double>();
  x.shearAxis = j.at("shearAxis").get<double>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
}
void to_json(json& j, const PatternLayerInstance& x)
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

  x.patternClass = j.at("class").get<PatternClass>();
  x.instance = j.at("instance").get<PatternInstanceType>();
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
  x.borderClass = j.at("class").get<BorderClass>();
  x.color = get_stack_optional<Color>(j, "color");
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.dashedOffset = j.at("dashedOffset").get<double>();
  x.dashedPattern = j.at("dashedPattern").get<std::vector<double>>();
  x.fillType = j.at("fillType").get<int64_t>();
  x.flat = j.at("flat").get<double>();
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  x.isEnabled = j.at("isEnabled").get<bool>();
  x.leftWeight = get_stack_optional<double>(j, "leftWeight");
  x.lineCapStyle = j.at("lineCapStyle").get<int64_t>();
  x.lineJoinStyle = j.at("lineJoinStyle").get<int64_t>();
  x.miterLimit = j.at("miterLimit").get<double>();
  x.pattern = get_stack_optional<Pattern>(j, "pattern");
  x.position = j.at("position").get<int64_t>();
  x.rightWeight = get_stack_optional<double>(j, "rightWeight");
  x.style = j.at("style").get<int64_t>();
  x.thickness = j.at("thickness").get<double>();
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
  x.fillClass = j.at("class").get<FillClass>();
  x.color = get_stack_optional<Color>(j, "color");
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.fillType = j.at("fillType").get<int64_t>();
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  x.isEnabled = j.at("isEnabled").get<bool>();
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
  x.fontVariationClass = j.at("class").get<FontVariationClass>();
  x.name = j.at("name").get<std::string>();
  x.value = j.at("value").get<double>();
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
  x.textParagraphClass = j.at("class").get<TextParagraphClass>();
  x.paragraphSpacing = j.at("paragraphSpacing").get<double>();
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
  x.textFontAttributesClass = j.at("class").get<FontAttrClass>();
  x.fills = get_stack_optional<std::vector<Fill>>(j, "fills");
  x.fillUseType = get_stack_optional<int64_t>(j, "fillUseType");
  x.fontVariantCaps = get_stack_optional<int64_t>(j, "fontVariantCaps");
  x.fontVariantPosition = get_stack_optional<int64_t>(j, "fontVariantPosition");
  x.fontVariations = get_stack_optional<std::vector<FontVariation>>(j, "fontVariations");
  x.horizontalScale = get_stack_optional<double>(j, "horizontalScale");
  x.hyperlink = get_stack_optional<std::string>(j, "hyperlink");
  x.length = get_stack_optional<int64_t>(j, "length");
  x.letterSpacingUnit = get_stack_optional<int64_t>(j, "letterSpacingUnit");
  x.letterSpacingValue = get_stack_optional<double>(j, "letterSpacingValue");
  x.lineSpacingUnit = get_stack_optional<int64_t>(j, "lineSpacingUnit");
  x.lineSpacingValue = get_stack_optional<double>(j, "lineSpacingValue");
  x.linethrough = get_stack_optional<bool>(j, "linethrough");
  x.name = get_stack_optional<std::string>(j, "name");
  x.postScript = get_stack_optional<std::string>(j, "postScript");
  x.rotate = get_stack_optional<double>(j, "rotate");
  x.size = get_stack_optional<double>(j, "size");
  x.subFamilyName = get_stack_optional<std::string>(j, "subFamilyName");
  x.textCase = get_stack_optional<int64_t>(j, "textCase");
  x.textParagraph = get_stack_optional<TextParagraph>(j, "textParagraph");
  x.underline = get_stack_optional<int64_t>(j, "underline");
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
  x.overrideValueClass = j.at("class").get<OverrideValueClass>();
  x.effectOnLayout = get_stack_optional<bool>(j, "effectOnLayout");
  x.objectId = j.at("objectId").get<std::vector<std::string>>();
  x.overrideName = j.at("overrideName").get<std::string>();
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
  x.pointAttrClass = j.at("class").get<PointClass>();
  x.cornerStyle = get_stack_optional<int64_t>(j, "cornerStyle");
  x.curveFrom = get_stack_optional<std::vector<double>>(j, "curveFrom");
  x.curveTo = get_stack_optional<std::vector<double>>(j, "curveTo");
  x.markType = get_stack_optional<int64_t>(j, "markType");
  x.point = j.at("point").get<std::vector<double>>();
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
  x.regionClass = j.at("class").get<RegionClass>();
  x.fills = get_stack_optional<std::vector<Fill>>(j, "fills");
  x.loops = j.at("loops").get<std::vector<std::vector<nlohmann::json>>>();
  x.windingRule = j.at("windingRule").get<int64_t>();
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
  x.segmentClass = j.at("class").get<SegmentClass>();
  x.curveFrom = get_stack_optional<std::vector<double>>(j, "curveFrom");
  x.curveTo = get_stack_optional<std::vector<double>>(j, "curveTo");
  x.end = j.at("end").get<int64_t>();
  x.start = j.at("start").get<int64_t>();
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
  x.center = j.at("center").get<std::vector<double>>();
  x.blurClass = j.at("class").get<BlurClass>();
  x.isEnabled = j.at("isEnabled").get<bool>();
  x.motionAngle = get_stack_optional<double>(j, "motionAngle");
  x.radius = get_stack_optional<double>(j, "radius");
  x.saturation = j.at("saturation").get<double>();
  x.type = j.at("type").get<int64_t>();
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
  x.blur = j.at("blur").get<double>();
  x.shadowClass = j.at("class").get<ShadowClass>();
  x.color = j.at("color").get<Color>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.inner = j.at("inner").get<bool>();
  x.isEnabled = j.at("isEnabled").get<bool>();
  x.offsetX = j.at("offsetX").get<double>();
  x.offsetY = j.at("offsetY").get<double>();
  x.showBehindTransparentAreas = get_stack_optional<bool>(j, "showBehindTransparentAreas");
  x.spread = j.at("spread").get<double>();
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
  x.blurs = j.at("blurs").get<std::vector<Blur>>();
  x.borders = j.at("borders").get<std::vector<Border>>();
  x.styleClass = j.at("class").get<StyleClass>();
  x.fills = j.at("fills").get<std::vector<Fill>>();
  x.shadows = j.at("shadows").get<std::vector<Shadow>>();
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
  x.textLineTypeClass = j.at("class").get<TextLineTypeClass>();
  x.isFirst = j.at("isFirst").get<bool>();
  x.level = j.at("level").get<int64_t>();
  x.styleType = j.at("styleType").get<int64_t>();
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
  x.textOnPathClass = j.at("class").get<TextOnPathClass>();
}

inline void to_json(json& j, const TextOnPath& x)
{
  j = json::object();
  j["class"] = x.textOnPathClass;
}

inline void from_json(const json& j, VariableAssign& x)
{
  x.variableAssignClass = j.at("class").get<VariableAssignmentClass>();
  x.id = j.at("id").get<std::string>();
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
  x.variableDefineClass = j.at("class").get<VariableDefClass>();
  x.id = j.at("id").get<std::string>();
  x.value = get_untyped(j, "value");
  x.varType = j.at("varType").get<int64_t>();
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
  x.variableReferClass = j.at("class").get<VariableRefClass>();
  x.id = j.at("id").get<std::string>();
  x.objectField = j.at("objectField").get<std::string>();
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
  x.vertexClass = j.at("class").get<VertexClass>();
  x.markType = get_stack_optional<int64_t>(j, "markType");
  x.point = j.at("point").get<std::vector<double>>();
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
  x.class_ = j.at("class").get<SubGeometryClass>();
  x.closed = j.at("closed").get<bool>();
  x.points = j.at("points").get<std::vector<PointAttr>>();
}

inline void to_json(json& j, const Contour& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["closed"] = x.closed;
  j["points"] = x.points;
}

void from_json(const json& j, VectorNetwork& x)
{
  x.class_ = j.at("class").get<SubGeometryClass>();
  x.vertices = j.at("vertices").get<std::vector<Vertex>>();
  x.segments = j.at("segments").get<std::vector<Segment>>();
  x.regions = j.at("regions").get<std::vector<Region>>();
}
void to_json(json& j, const VectorNetwork& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["vertices"] = x.vertices;
  j["segments"] = x.segments;
  j["regions"] = x.regions;
}

void from_json(const json& j, Ellipse& x)
{
  x.class_ = j.at("class").get<SubGeometryClass>();
  x.startingAngle = j.at("startingAngle").get<double>();
  x.endingAngle = j.at("endingAngle").get<double>();
  x.innerRadius = j.at("innerRadius").get<double>();
}
void to_json(json& j, const Ellipse& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["startingAngle"] = x.startingAngle;
  j["endingAngle"] = x.endingAngle;
  j["innerRadius"] = x.innerRadius;
}

void from_json(const json& j, Polygon& x)
{
  x.class_ = j.at("class").get<SubGeometryClass>();
  x.pointCount = j.at("pointCount").get<int>();
  x.radius = get_stack_optional<double>(j, "radius");
}
void to_json(json& j, const Polygon& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["pointCount"] = x.pointCount;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

void from_json(const json& j, Rectangle& x)
{
  x.class_ = j.at("class").get<SubGeometryClass>();
  x.cornerRadius = get_stack_optional<double>(j, "cornerRadius");
  x.radius = get_stack_optional<std::vector<double>>(j, "radius");
}
void to_json(json& j, const Rectangle& x)
{
  j = json::object();
  j["class"] = x.class_;
  j["cornerRadius"] = x.cornerRadius;
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
}

void from_json(const json& j, Star& x)
{
  x.class_ = j.at("class").get<SubGeometryClass>();
  x.ratio = j.at("ratio").get<double>();
  x.pointCount = j.at("pointCount").get<int>();
  x.radius = get_stack_optional<double>(j, "radius");
}
void to_json(json& j, const Star& x)
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
  x.booleanOperation = j.at("booleanOperation").get<int64_t>();
  x.subshapeClass = j.at("class").get<SubshapeClass>();

  x.subGeometry = j.at("subGeometry").get<std::shared_ptr<SubGeometryType>>();
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
  x.shapeClass = j.at("class").get<ShapeClass>();
  x.radius = get_stack_optional<double>(j, "radius");
  x.subshapes = j.at("subshapes").get<std::vector<Subshape>>();
  x.windingRule = j.at("windingRule").get<int64_t>();
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

inline void from_json(const json& j, Path& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.pathClass = j.at("class").get<ChildObjectClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.shape = get_heap_optional<Shape>(j, "shape");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
  x.fillReplacesImage = get_stack_optional<bool>(j, "fillReplacesImage");
  x.imageFileName = get_stack_optional<std::string>(j, "imageFileName");
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
  x.anchorPoint = get_stack_optional<std::vector<double>>(j, "anchorPoint");
  x.content = get_stack_optional<std::string>(j, "content");
  x.defaultFontAttr = get_stack_optional<TextFontAttributes>(j, "defaultFontAttr");
  x.fontAttr = get_stack_optional<std::vector<TextFontAttributes>>(j, "fontAttr");
  x.frameMode = get_stack_optional<int64_t>(j, "frameMode");
  x.horizontalAlignment = get_stack_optional<std::vector<int64_t>>(j, "horizontalAlignment");
  x.textLineType = get_stack_optional<std::vector<TextLineType>>(j, "textLineType");
  x.textOnPath = get_stack_optional<TextOnPath>(j, "textOnPath");
  x.truncatedHeight = get_stack_optional<double>(j, "truncatedHeight");
  x.verticalAlignment = get_stack_optional<int64_t>(j, "verticalAlignment");
  x.verticalTrim = get_stack_optional<bool>(j, "verticalTrim");
  x.childObjects = get_stack_optional<std::vector<Path>>(j, "childObjects");
  x.groupNestMaskType = get_stack_optional<bool>(j, "groupNestMaskType");
  x.isVectorNetwork = get_stack_optional<bool>(j, "isVectorNetwork");
  x.backgroundColor = get_stack_optional<Color>(j, "backgroundColor");
  x.radius = get_stack_optional<std::vector<double>>(j, "radius");
  x.masterId = get_stack_optional<std::string>(j, "masterId");
  x.overrideValues = get_stack_optional<std::vector<OverrideValue>>(j, "overrideValues");
  x.variableAssignments = get_stack_optional<std::vector<VariableAssign>>(j, "variableAssignments");
}

inline void to_json(json& j, const Path& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["class"] = x.pathClass;
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
  if (x.resizesContent)
  {
    j["resizesContent"] = x.resizesContent;
  }
  if (x.shape)
  {
    j["shape"] = x.shape;
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
  if (x.fillReplacesImage)
  {
    j["fillReplacesImage"] = x.fillReplacesImage;
  }
  if (x.imageFileName)
  {
    j["imageFileName"] = x.imageFileName;
  }
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
  if (x.anchorPoint)
  {
    j["anchorPoint"] = x.anchorPoint;
  }
  if (x.content)
  {
    j["content"] = x.content;
  }
  if (x.defaultFontAttr)
  {
    j["defaultFontAttr"] = x.defaultFontAttr;
  }
  if (x.fontAttr)
  {
    j["fontAttr"] = x.fontAttr;
  }
  if (x.frameMode)
  {
    j["frameMode"] = x.frameMode;
  }
  if (x.horizontalAlignment)
  {
    j["horizontalAlignment"] = x.horizontalAlignment;
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
  if (x.verticalAlignment)
  {
    j["verticalAlignment"] = x.verticalAlignment;
  }
  if (x.verticalTrim)
  {
    j["verticalTrim"] = x.verticalTrim;
  }
  if (x.childObjects)
  {
    j["childObjects"] = x.childObjects;
  }
  if (x.groupNestMaskType)
  {
    j["groupNestMaskType"] = x.groupNestMaskType;
  }
  if (x.isVectorNetwork)
  {
    j["isVectorNetwork"] = x.isVectorNetwork;
  }
  if (x.backgroundColor)
  {
    j["backgroundColor"] = x.backgroundColor;
  }
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
  if (x.masterId)
  {
    j["masterId"] = x.masterId;
  }
  if (x.overrideValues)
  {
    j["overrideValues"] = x.overrideValues;
  }
  if (x.variableAssignments)
  {
    j["variableAssignments"] = x.variableAssignments;
  }
}

inline void from_json(const json& j, Frame& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.backgroundColor = get_stack_optional<Color>(j, "backgroundColor");
  x.bounds = j.at("bounds").get<Rect>();
  x.childObjects = j.at("childObjects").get<std::vector<ContainerChildType>>();
  x.frameClass = j.at("class").get<FrameClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.radius = get_stack_optional<std::vector<double>>(j, "radius");
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
}

inline void to_json(json& j, const Frame& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  if (x.backgroundColor)
  {
    j["backgroundColor"] = x.backgroundColor;
  }
  j["bounds"] = x.bounds;
  j["childObjects"] = x.childObjects;
  j["class"] = x.frameClass;
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
  if (x.radius)
  {
    j["radius"] = x.radius;
  }
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

inline void from_json(const json& j, Group& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.childObjects = j.at("childObjects").get<std::vector<ContainerChildType>>();
  x.class_ = j.at("class").get<ChildObjectClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.groupNestMaskType = get_stack_optional<bool>(j, "groupNestMaskType");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.isVectorNetwork = get_stack_optional<bool>(j, "isVectorNetwork");
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
}

inline void to_json(json& j, const Group& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["childObjects"] = x.childObjects;
  j["class"] = x.class_;
  j["contextSettings"] = x.contextSettings;
  if (x.cornerSmoothing)
  {
    j["cornerSmoothing"] = x.cornerSmoothing;
  }
  if (x.groupNestMaskType)
  {
    j["groupNestMaskType"] = x.groupNestMaskType;
  }
  if (x.horizontalConstraint)
  {
    j["horizontalConstraint"] = x.horizontalConstraint;
  }
  j["id"] = x.id;
  j["isLocked"] = x.isLocked;
  if (x.isVectorNetwork)
  {
    j["isVectorNetwork"] = x.isVectorNetwork;
  }
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

void from_json(const json& j, Image& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.class_ = j.at("class").get<ChildObjectClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.fillReplacesImage = get_stack_optional<bool>(j, "fillReplacesImage");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.imageFileName = j.at("imageFileName").get<std::string>();
  x.imageFilters = get_stack_optional<ImageFilters>(j, "imageFilters");
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
}
void to_json(json& j, const Image& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["class"] = x.class_;
  j["contextSettings"] = x.contextSettings;
  if (x.cornerSmoothing)
  {
    j["cornerSmoothing"] = x.cornerSmoothing;
  }
  if (x.fillReplacesImage)
  {
    j["fillReplacesImage"] = x.fillReplacesImage;
  }
  if (x.horizontalConstraint)
  {
    j["horizontalConstraint"] = x.horizontalConstraint;
  }
  j["id"] = x.id;
  j["imageFileName"] = x.imageFileName;
  if (x.imageFilters)
  {
    j["imageFilters"] = x.imageFilters;
  }
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

void from_json(const json& j, SymbolInstance& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.class_ = j.at("class").get<ChildObjectClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.masterId = j.at("masterId").get<std::string>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.overrideValues = j.at("overflow").get<std::vector<OverrideValue>>();
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableAssignments = get_stack_optional<std::vector<VariableAssign>>(j, "variableAssignments");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
}
void to_json(json& j, const SymbolInstance& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["class"] = x.class_;
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
  j["masterId"] = x.masterId;
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
  j["overrideValues"] = x.overrideValues;
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
  if (x.variableAssignments)
  {
    j["variableAssignments"] = x.variableAssignments;
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

void from_json(const json& j, SymbolMaster& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.childObjects = j.at("childObjects").get<std::vector<ContainerChildType>>();
  x.class_ = j.at("class").get<ChildObjectClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.radius = j.at("radius").get<std::vector<double>>();
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
}
void to_json(json& j, const SymbolMaster& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  j["bounds"] = x.bounds;
  j["childObjects"] = x.childObjects;
  j["class"] = x.class_;
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
  j["radius"] = x.radius;
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

void from_json(const json& j, Text& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.anchorPoint = get_stack_optional<std::vector<double>>(j, "anchorPoint");
  x.bounds = j.at("bounds").get<Rect>();
  x.class_ = j.at("class").get<ChildObjectClass>();
  x.content = j.at("content").get<std::string>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.defaultFontAttr = get_stack_optional<TextFontAttributes>(j, "defaultFontAttr");
  x.fontAttr = j.at("fontAttr").get<std::vector<TextFontAttributes>>();
  x.frameMode = j.at("frameMode").get<int>();
  x.horizontalAlignment = j.at("horizontalAlignment").get<std::vector<int>>();
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.textLineType = get_stack_optional<std::vector<TextLineType>>(j, "textLineType");
  x.textOnPath = get_stack_optional<TextOnPath>(j, "textOnPath");
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.truncatedHeight = get_stack_optional<double>(j, "truncatedHeight");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalAlignment = j.at("verticalAlignment").get<int64_t>();
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.verticalTrim = get_stack_optional<bool>(j, "verticalTrim");
  x.visible = j.at("visible").get<bool>();
}
void to_json(json& j, const Text& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alphaMaskBy;
  if (x.anchorPoint)
  {
    j["anchorPoint"] = x.anchorPoint;
  }
  j["bounds"] = x.bounds;
  j["class"] = x.class_;
  j["content"] = x.content;
  j["contextSettings"] = x.contextSettings;
  if (x.cornerSmoothing)
  {
    j["cornerSmoothing"] = x.cornerSmoothing;
  }
  if (x.defaultFontAttr)
  {
    j["defaultFontAttr"] = x.defaultFontAttr;
  }
  j["fontAttr"] = x.fontAttr;
  j["frameMode"] = x.frameMode;
  j["horizontalAlignment"] = x.horizontalAlignment;
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
  if (x.textLineType)
  {
    j["textLineType"] = x.textLineType;
  }
  if (x.textOnPath)
  {
    j["textOnPath"] = x.textOnPath;
  }
  if (x.transformedBounds)
  {
    j["transformedBounds"] = x.transformedBounds;
  }
  if (x.truncatedHeight)
  {
    j["truncatedHeight"] = x.truncatedHeight;
  }
  if (x.variableDefs)
  {
    j["variableDefs"] = x.variableDefs;
  }
  if (x.variableRefs)
  {
    j["variableRefs"] = x.variableRefs;
  }
  j["verticalAlignment"] = x.verticalAlignment;
  if (x.verticalConstraint)
  {
    j["verticalConstraint"] = x.verticalConstraint;
  }
  if (x.verticalTrim)
  {
    j["verticalTrim"] = x.verticalTrim;
  }
  j["visible"] = x.visible;
}

inline void from_json(const json& j, PatternLayerDef& x)
{
  x.alphaMaskBy = j.at("alphaMaskBy").get<std::vector<AlphaMask>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.childObjects = j.at("childObjects").get<std::vector<Path>>();
  x.patternLayerDefClass = j.at("class").get<PatternLayerDefClass>();
  x.contextSettings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.cornerSmoothing = get_stack_optional<double>(j, "cornerSmoothing");
  x.horizontalConstraint = get_stack_optional<int64_t>(j, "horizontalConstraint");
  x.id = j.at("id").get<std::string>();
  x.isLocked = j.at("isLocked").get<bool>();
  x.keepShapeWhenResize = get_stack_optional<bool>(j, "keepShapeWhenResize");
  x.maskShowType = get_stack_optional<int64_t>(j, "maskShowType");
  x.maskType = j.at("maskType").get<int64_t>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outlineMaskBy = j.at("outlineMaskBy").get<std::vector<std::string>>();
  x.overflow = j.at("overflow").get<int64_t>();
  x.overrideKey = get_stack_optional<std::string>(j, "overrideKey");
  x.patternBoundingBox = j.at("patternBoundingBox").get<std::vector<double>>();
  x.resizesContent = get_stack_optional<int64_t>(j, "resizesContent");
  x.style = j.at("style").get<Style>();
  x.styleEffectBoolean = get_stack_optional<int64_t>(j, "styleEffectBoolean");
  x.styleEffectMaskArea = j.at("styleEffectMaskArea").get<int64_t>();
  x.transformedBounds = get_stack_optional<Rect>(j, "transformedBounds");
  x.variableDefs = get_stack_optional<std::vector<VariableDefine>>(j, "variableDefs");
  x.variableRefs = get_stack_optional<std::vector<VariableRefer>>(j, "variableRefs");
  x.verticalConstraint = get_stack_optional<int64_t>(j, "verticalConstraint");
  x.visible = j.at("visible").get<bool>();
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
  x.class_ = j.at("class").get<ReferenceClass>();
  x.id = j.at("id").get<std::string>();
  x.style = j.at("style").get<Style>();
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
  x.fileType = j.at("fileType").get<int64_t>();
  x.frames = j.at("frames").get<std::vector<Frame>>();
  x.patternLayerDef = get_stack_optional<std::vector<PatternLayerDef>>(j, "patternLayerDef");
  x.references = get_stack_optional<std::vector<ReferenceType>>(j, "references");
  x.version = j.at("version").get<Version>();
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
    throw std::runtime_error("Input JSON does not conform to schema!");
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
      throw std::runtime_error("This should not happen");
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

inline void from_json(const json& j, ChildObjectClass& x)
{
  if (j == "frame")
    x = ChildObjectClass::FRAME;
  else if (j == "group")
    x = ChildObjectClass::GROUP;
  else if (j == "image")
    x = ChildObjectClass::IMAGE;
  else if (j == "path")
    x = ChildObjectClass::PATH;
  else if (j == "symbolInstance")
    x = ChildObjectClass::SYMBOL_INSTANCE;
  else if (j == "symbolMaster")
    x = ChildObjectClass::SYMBOL_MASTER;
  else if (j == "text")
    x = ChildObjectClass::TEXT;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const ChildObjectClass& x)
{
  switch (x)
  {
    case ChildObjectClass::FRAME:
      j = "frame";
      break;
    case ChildObjectClass::GROUP:
      j = "group";
      break;
    case ChildObjectClass::IMAGE:
      j = "image";
      break;
    case ChildObjectClass::PATH:
      j = "path";
      break;
    case ChildObjectClass::SYMBOL_INSTANCE:
      j = "symbolInstance";
      break;
    case ChildObjectClass::SYMBOL_MASTER:
      j = "symbolMaster";
      break;
    case ChildObjectClass::TEXT:
      j = "text";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}

inline void from_json(const json& j, FrameClass& x)
{
  if (j == "frame")
    x = FrameClass::FRAME;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const FrameClass& x)
{
  switch (x)
  {
    case FrameClass::FRAME:
      j = "frame";
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

inline void from_json(const json& j, Version& x)
{
  if (j == "1.0.16")
    x = Version::THE_1016;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const Version& x)
{
  switch (x)
  {
    case Version::THE_1016:
      j = "1.0.16";
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

  auto classType = j.at("class").get<ChildObjectClass>();
  switch (classType)
  {
    case ChildObjectClass::FRAME:
      x = j.get<Frame>();
      break;
    case ChildObjectClass::GROUP:
      x = j.get<Group>();
      break;
    case ChildObjectClass::IMAGE:
      x = j.get<Image>();
      break;
    case ChildObjectClass::PATH:
      x = j.get<Path>();
      break;
    case ChildObjectClass::SYMBOL_INSTANCE:
      x = j.get<SymbolInstance>();
      break;
    case ChildObjectClass::SYMBOL_MASTER:
      x = j.get<SymbolMaster>();
      break;
    case ChildObjectClass::TEXT:
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
  else
  {
    throw std::runtime_error("Could not serialise, invalid node!");
  }
}
} // namespace nlohmann
