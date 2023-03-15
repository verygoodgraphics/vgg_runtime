//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     DesignDocumentModel data = nlohmann::json::parse(jsonString);

#pragma once

#include <optional>
#include <variant>
#include "nlohmann/json.hpp"

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
using nlohmann::json;

#ifndef NLOHMANN_UNTYPED_VGG_HELPER
#define NLOHMANN_UNTYPED_VGG_HELPER
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

#ifndef NLOHMANN_OPTIONAL_VGG_HELPER
#define NLOHMANN_OPTIONAL_VGG_HELPER
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

/**
 * Defines a RGBA color value.
 *
 * The border color of the font.If it doesn't exist it means it doesn't need to be stroked.
 *
 * The fill color of the font.If it doesn't exist it means it doesn't need to be filled.
 *
 * The color of the shadow.
 */
struct Color
{
  double alpha;
  double blue;
  nlohmann::json color_class;
  double green;
  double red;
};

/**
 * This property records the bounds information of the object before it is matrixed. For
 * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
 * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
 * -height), (0, -height) } + matrix_transformation = frame.
 *
 * Defines an abstract rectangle.
 *
 * Frame information for the object. When the object is a group, its frame is the union of
 * all objects in the group, whether or not the group contains a mask. This property stores
 * information about the object after the matrix transformation.
 */
struct Rect
{
  nlohmann::json rect_class;
  /**
   * Whether to keep the height and width ratio constant while scaling.
   */
  bool constrain_proportions;
  double height;
  double width;
  /**
   * Top left X coordinate. The X value in the top left corner is 0. The X-axis increases to
   * the right.
   */
  double x;
  /**
   * Top left Y coordinate. The Y value in the top left corner is 0. The Y-axis increases to
   * the up.
   */
  double y;
};

/**
 * This property of path itself is meaningless, and is represented by its style, such as
 * stroke, fill, etc.
 *
 * Defines the opacity and blend mode and isolate blending and transparency knockout group
 * of a object.
 */
struct GraphicsContextSettings
{
  int64_t blend_mode;
  nlohmann::json graphics_context_settings_class;
  /**
   * You can isolate the blending mode to a targeted layer or group in order to leave objects
   * beneath unaffected.
   */
  bool isolate_blending;
  double opacity;
  /**
   * In a transparency knockout group, the elements of a group don't show through each other.
   * 0: OFF 1: ON 2: Neutral
   */
  int64_t transparency_knockout_group;
};

/**
 * Describes the paragraph properties of the text.
 */
struct TextParagraph
{
  nlohmann::json text_paragraph_class;
  double paragraph_spacing;
};

/**
 * Text Attr defined for particular fragments of the text (text ranges).
 */
struct TextAttr
{
  /**
   * Describes baseline's position type. 0: normal 1: superScript 2: subScript
   */
  int64_t baseline;
  /**
   * Describes baselineShift value.
   */
  double baseline_shift;
  /**
   * Describes if the text is bold. If missing the information is not known.
   */
  std::optional<bool> bold;
  /**
   * The border color of the font.If it doesn't exist it means it doesn't need to be stroked.
   */
  std::optional<Color> border_color;
  /**
   * Describes the stroke width.Exists only when stroke is required.
   */
  std::optional<double> border_size;
  nlohmann::json text_attr_class;
  GraphicsContextSettings context_settings;
  /**
   * The fill color of the font.If it doesn't exist it means it doesn't need to be filled.
   */
  std::optional<Color> fill_color;
  /**
   * Horizontal alignment of the text. 0: left 1: center 2: right 3: justify
   */
  int64_t horizontal_alignment;
  /**
   * Describes the horizontal scaling value of text.
   * Value example:
   * 1:    not scale
   * 1.75: 175%
   * 2:    200%
   * 0.5:  50%
   */
  double horizontal_scale;
  /**
   * Describes if the text is italic. If missing the information is not known.
   */
  std::optional<bool> italic;
  /**
   * Describes if kerning feature is enabled in the text.
   */
  bool kerning;
  /**
   * The number of characters(utf-8) that the current attribute acts on.
   */
  int64_t length;
  /**
   * Text letter spacing (can be negative).
   */
  double letter_spacing;
  /**
   * 0: do nothing 1: open smallcaps 2: open uppercase 3: open lowercase
   */
  int64_t lettertransform;
  /**
   * Describes the line spacing values of the text.
   */
  double line_space;
  /**
   * Describes if the text has line through.
   */
  bool linethrough;
  /**
   * Common name of font. The font name may contain subfamily, in which case the subFamilyName
   * will not appear.
   */
  std::string name;
  /**
   * Describes the proportional spacing of text.
   * Value example:
   * 1:    100%
   * 0.5:  50%
   * 0:    0%
   */
  double proportional_spacing;
  /**
   * Describes the value of text rotation(in degree).
   */
  double rotate;
  /**
   * Font's size.
   */
  double size;
  /**
   * Font type (called 'subfamily' in OpenType specification).
   */
  std::optional<std::string> sub_family_name;
  TextParagraph text_paragraph;
  /**
   * Text underline type. 0: none 1: single line 2: double line
   */
  int64_t underline;
  /**
   * Describes the vertical scaling value of text. Value example reference horizontalScale.
   */
  double vertical_scale;
};

struct Data
{
  std::string data;
};

enum class RefClass : int
{
  MS_FONT_DATA,
  MS_IMAGE_DATA,
  MS_IMMUTABLE_PAGE,
  MS_PATCH
};

struct Sha1
{
  std::string data;
};

/**
 * Defines a reference to a file within the document bundle.
 *
 * Defines inline base64 data.
 */
struct Reference
{
  std::string ref;
  RefClass ref_class;
  nlohmann::json reference_class;
  std::optional<Data> data;
  std::optional<Sha1> sha1;
};

using Value = std::variant<Reference, std::string>;

/**
 * Defines an individual symbol override.
 */
struct OverrideValue
{
  nlohmann::json override_value_class;
  std::optional<std::string> do_object_id;
  std::string override_name;
  Value value;
};

/**
 * A single point descriptor.
 */
struct PointAttr
{
  nlohmann::json point_attr_class;
  /**
   * If the point is of type cubic, the items are stored as c1, c2, p, where p is the point on
   * the curve and c1 and c2 are its neighbouring control points. (In sketch, c1 eauql
   * curveto, c2 equal curvefrom) For type quad the format is c, p. for type simple point it
   * is p.
   */
  std::vector<std::vector<double>> points;
  /**
   * Describes a type of the point. 0: simple point 1: quad 2: cubic
   */
  int64_t point_type;
  /**
   * Takes effect only in simple point mode, indicates that the effect of the current point is
   * an arc, and specifies the radius of the arc.
   */
  std::optional<double> radius;
};

/**
 * The meaning of the gradient vector is different for radial gradients than for linear
 * gradients. The vector origin is the center of the circle containing the radial gradient;
 * the vector length is the radius of the that circle. The vector angle is not used by
 * radial blends, but is preserved and used if the user changes the gradient from radial to
 * linear.
 *
 * Gradient geometry defines much of the appearance of the gradient within the path.
 */
struct GradientBasicGeometry
{
  /**
   * This argument specifies the direction of the gradient vector, in degrees. The gradient
   * ramp extends from the origin at the value of angle, which is measured counterclockwise
   * from the x axis.
   */
  double angle;
  nlohmann::json gradient_basic_geometry_class;
  /**
   * This argument defines how the gradient will be rendered.For simple filled paths, flag
   * takes the value 1. 0: Do not issue a clip. 1: Issue a clip. 2: Disable rendering.
   */
  int64_t flag;
  /**
   * This argument specifies the distance over which the gradient ramp is applied. The ramp
   * will be scaled so that its 100% value is the end of the length. This parameter may be
   * greater than 1.
   */
  double length;
  /**
   * The six values make up a transformation matrix. When a gradient is first applied to an
   * object, these values are the identity matrix. If the user transforms the object, the user
   * transformation matrix is concatenated to the gradient instance’s matrix.
   */
  std::vector<double> matrix;
  /**
   * 0.5 Indicates that the width is half the length. 2 Indicates that the width is twice the
   * length. and so on.
   */
  double width_ratio;
  /**
   * xOrigin and yOrigin give the origin of the gradient in page coordinates. The origin can
   * be located anywhere on the artwork, and corresponds to 0 on the gradient ramp.
   */
  double x_origin;
  /**
   * Refer to xOriginal for details.
   */
  double y_origin;
};

/**
 * Only exists in radial mode.
 *
 * Radial gradients have an additional attribute called a hilight. The hilight serves at the
 * starting point for the gradient ramp as it expands outward. It is still contained within
 * the gradient vector circle.
 */
struct GradientHilight
{
  /**
   * This argument is the angle (in degrees) to the hilight point, measured counterclockwise
   * from the x axis.
   */
  double angle;
  nlohmann::json gradient_hilight_class;
  /**
   * This argument is the distance of the hilight from the origin, expressed as a fraction of
   * the radius—a value between 0 and 1.
   */
  double length;
  /**
   * xHilight and yHilight specify the hilight placement, in x and y offsets from the gradient
   * vector origin.
   */
  double x_hilight;
  /**
   * Refer to xHilight for details.
   */
  double y_hilight;
};

/**
 * Defines a position on a gradient that marks the end of a transition to a new color.
 */
struct GradientStop
{
  nlohmann::json gradient_stop_class;
  Color color;
  /**
   * Specifies the location between two ramp points where there is an equal mix of the two
   * colors. midPoint is a percentage of the distance between two ramp points. The mid point
   * for the last color stop is ignored.
   */
  double mid_point;
  /**
   * A number giving the position of a color stop on the gradient ramp.
   */
  double position;
};

/**
 * The current gradient instance.
 *
 * Described parameters required by gradient angular.
 *
 * Described parameters required by gradient linear.
 *
 * Described parameters required by gradient radial.
 *
 * Describes the parameters of gradients in a more basic form.
 */
struct InstanceElement
{
  nlohmann::json gradient_class;
  std::optional<std::vector<double>> from;
  /**
   * Flag describing if gradient has been inverted.
   */
  bool invert;
  /**
   * Describes the Angle of the starting position. Value of rotation in degrees.
   */
  std::optional<double> rotation;
  std::vector<GradientStop> stops;
  std::optional<std::vector<double>> to;
  /**
   * For elliptic gradients describes ratio of major to minor semi-axis of the ellipse.
   */
  std::optional<double> elipse_length;
  /**
   * The meaning of the gradient vector is different for radial gradients than for linear
   * gradients. The vector origin is the center of the circle containing the radial gradient;
   * the vector length is the radius of the that circle. The vector angle is not used by
   * radial blends, but is preserved and used if the user changes the gradient from radial to
   * linear.
   */
  std::optional<GradientBasicGeometry> geometry;
  /**
   * 0: linear gradient 1: radial gradient
   */
  std::optional<int64_t> gradient_type;
  /**
   * Only exists in radial mode.
   */
  std::optional<GradientHilight> hilight;
  /**
   * Its parameters are six floating point values, which describe the overall matrix applied
   * to the gradient.
   */
  std::optional<std::vector<double>> overall_matrix;
  std::optional<std::vector<nlohmann::json>> perpendicular_matrix;
};

using Instance = std::optional<std::variant<std::vector<InstanceElement>,
                                            bool,
                                            double,
                                            int64_t,
                                            std::map<std::string, nlohmann::json>,
                                            std::string>>;

/**
 * Defines a gradient.
 */
struct Gradient
{
  nlohmann::json gradient_class;
  /**
   * The current gradient instance.
   */
  Instance instance;
};

/**
 * Use an image as the content of pattern.
 *
 * Use an layer as the content of pattern.
 */
struct InstanceClass
{
  nlohmann::json pattern_class;
  std::optional<int64_t> fill_type;
  /**
   * image filename.
   */
  std::optional<std::string> image_file_name;
  /**
   * Whether to mirror the image.
   */
  std::optional<bool> image_tile_mirrored;
  /**
   * Image scale value in tile mode.
   */
  std::optional<double> image_tile_scale;
  /**
   * Specifies the angle in counterclockwise degrees to rotate the pattern.
   */
  std::optional<double> angle;
  /**
   * Specifies the initial matrix to which all other pattern transformations are to be
   * applied. This matrix describes transformations that are not otherwise expressible as the
   * single combination of the other transformations.
   */
  std::optional<std::vector<double>> matrix;
  /**
   * Specify the offset from the ruler origin of the origin to be used for tiling the pattern.
   * Each distance specified in points.
   */
  std::optional<std::vector<double>> offset;
  /**
   * Specifies the angle of the line from the origin about which the pattern is reflected.
   * Used if the reflection operand is true.
   */
  std::optional<double> r;
  /**
   * The name(utf-8) of the referenced pattern layer. Note that this is the name of the
   * pattern layer, not its id. The pattern layer name is unique.
   */
  std::optional<std::string> ref_layer_name;
  /**
   * Flag indicating whether to apply a reflection to the pattern.
   */
  std::optional<bool> reflection;
  /**
   * Specify the scale factors to be applied to the x and y dimensions, respectively, of the
   * pattern.
   */
  std::optional<std::vector<double>> scale;
  /**
   * Specifies the shear angle.
   */
  std::optional<double> shear;
  /**
   * Specifies the shear axis.
   */
  std::optional<double> shear_axis;
};

/**
 * Pattern descriptor.
 */
struct Pattern
{
  nlohmann::json pattern_class;
  InstanceClass instance;
};

/**
 * Defines a border style
 */
struct Border
{
  nlohmann::json border_class;
  std::optional<Color> color;
  GraphicsContextSettings context_settings;
  double dashed_offset;
  /**
   * A set of alternating dashes (filled regions) and gaps (empty regions). (applies to
   * 'dashed' borders)
   */
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
  /**
   * Type of the border style, the dashed style is further specified in dashedOffset and
   * dashedPattern. 0: solid 1: dotted 2: dashed
   */
  int64_t style;
  /**
   * Thickness of the border. A line width of 0 is acceptable, this is interpreted as the
   * thinnest line width that can be rendered at device resolution.
   */
  double thickness;
};

/**
 * Defines a shadow style
 */
struct Shadow
{
  /**
   * Blur value of the shadow.
   */
  double blur;
  nlohmann::json shadow_class;
  /**
   * The color of the shadow.
   */
  Color color;
  GraphicsContextSettings context_settings;
  /**
   * True means inner shadows. False means shadows.
   */
  bool inner;
  bool is_enabled;
  /**
   * Horizontal offset of the shadow.
   */
  double offset_x;
  /**
   * Vertical offset of the shadow.
   */
  double offset_y;
  /**
   * Spread amount of the shadow contour.
   */
  double spread;
};

/**
 * Object styles descriptor.
 *
 * Defines a object style.
 */
struct Style
{
  /**
   * An array containing the blur effects of the object.
   */
  std::vector<nlohmann::json> blurs;
  /**
   * An array containing the border styles of the object.
   */
  std::vector<Border> borders;
  nlohmann::json style_class;
  /**
   * An array containing the fill effects of the object.
   */
  std::vector<nlohmann::json> fills;
  /**
   * An array containing the shadow effects of the object.
   */
  std::vector<Shadow> shadows;
};

/**
 * If this item exists, the text is on the path and the matrix information for the text
 * should not be used.
 *
 * Describes information about text on a path.
 */
struct TextOnPath
{
  nlohmann::json text_on_path_class;
  /**
   * Id of the associated path.
   */
  std::string path_id;
  /**
   * The percentage of the text's starting position on the path.
   */
  double start_offset;
};

/**
 * The contour of the subpath is described.
 *
 * Describes the contour of a shape.
 *
 * Contains the information needed to draw the text.
 *
 * Contains the information needed to draw the image.
 *
 * Symbol instance object represent an instance of a symbol source.
 */
struct Contour
{
  nlohmann::json contour_class;
  /**
   * Describes if the path is opened or closed.
   */
  std::optional<bool> closed;
  std::optional<std::vector<PointAttr>> points;
  std::optional<std::vector<nlohmann::json>> alpha_mask_by;
  /**
   * An array containing font style ranges. Note: Arrays are ordered.
   */
  std::optional<std::vector<TextAttr>> attr;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  std::optional<Rect> bounds;
  /**
   * Text content of the text object. The encoding format is utf-8.
   */
  std::optional<std::string> content;
  /**
   * This property of path itself is meaningless, and is represented by its style, such as
   * stroke, fill, etc.
   */
  std::optional<GraphicsContextSettings> context_settings;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  std::optional<Rect> frame;
  /**
   * Specifies the text frame semantics. fixed mode strictly sets the typeset area according
   * to the frame. auto width and auto height mode expands associated bounds as needed. 0:
   * fixed 1: auto width 2: auto height
   */
  std::optional<int64_t> frame_mode;
  /**
   * Unique object identifier.
   * ID format:
   * Photoshop: numeric
   * Sketch: UUID string
   * Adobe XD: UUID string
   * Figma: string like 3:5:3
   * In Illustrator: numeric
   */
  std::optional<std::string> id;
  /**
   * Describes whether the object is locked.
   */
  std::optional<bool> is_locked;
  /**
   * Indicates whether the current object is a mask object.
   */
  std::optional<bool> is_mask;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::optional<std::vector<double>> matrix;
  /**
   * Name of object (utf-8).
   */
  std::optional<std::string> name;
  /**
   * The result of the outline mask area is the intersection of multiple outline mask object
   * results.
   */
  std::optional<std::vector<nlohmann::json>> outline_mask_by;
  /**
   * Object styles descriptor.
   */
  std::optional<Style> style;
  /**
   * If this item exists, the text is on the path and the matrix information for the text
   * should not be used.
   */
  std::optional<TextOnPath> text_on_path;
  /**
   * Text vertical alignment. 0: top 1: center 2: bottom
   */
  std::optional<int64_t> vertical_alignment;
  /**
   * Describes the visibility of the object.
   */
  std::optional<bool> visible;
  /**
   * The relative path to the image file (utf-8).
   */
  std::optional<std::string> image_file_name;
  std::optional<std::vector<OverrideValue>> override_values;
  std::optional<double> scale;
  /**
   * The id of the referenced symbol master.
   */
  std::optional<std::string> symbol_master_id;
};

/**
 * A single subshape descriptor.
 */
struct Subshape
{
  /**
   * Boolean operation combining this and next-in-the-array path. 0: union 1: subtraction 2:
   * intersecion 3: exclusion 4: none
   */
  int64_t boolean_operation;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  Rect bounds;
  nlohmann::json subshape_class;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  Rect frame;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::vector<double> matrix;
  /**
   * The contour of the subpath is described.
   */
  Contour sub_geometry;
};

/**
 * Specifies shape of the path.
 *
 * Shape descriptor.
 */
struct Shape
{
  nlohmann::json shape_class;
  /**
   * An array of subshape in the shape.
   */
  std::vector<Subshape> subshapes;
  /**
   * The value describes the shape's 'winding rule' (policy determining how overlapping
   * contours fill an area). 0: non-zero 1: even-odd
   */
  int64_t winding_rule;
};

/**
 * Path represents a vector geometry which is formed by individual subpaths combined
 * together via boolean operations.
 *
 * Contains the information needed to draw the image.
 *
 * Contains the information needed to draw the text.
 *
 * A group is a combination of a series of objects.
 *
 * Symbol instance object represent an instance of a symbol source.
 */
struct Path
{
  std::vector<nlohmann::json> alpha_mask_by;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  Rect bounds;
  nlohmann::json path_class;
  /**
   * This property of path itself is meaningless, and is represented by its style, such as
   * stroke, fill, etc.
   */
  GraphicsContextSettings context_settings;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  Rect frame;
  /**
   * Unique object identifier.
   * ID format:
   * Photoshop: numeric
   * Sketch: UUID string
   * Adobe XD: UUID string
   * Figma: string like 3:5:3
   * In Illustrator: numeric
   */
  std::string id;
  /**
   * Describes whether the object is locked.
   */
  bool is_locked;
  /**
   * Indicates whether the current object is a mask object.
   */
  bool is_mask;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::vector<double> matrix;
  /**
   * Name of object (utf-8).
   */
  std::optional<std::string> name;
  /**
   * The result of the outline mask area is the intersection of multiple outline mask object
   * results.
   */
  std::vector<nlohmann::json> outline_mask_by;
  /**
   * Specifies shape of the path.
   */
  std::optional<Shape> shape;
  /**
   * Object styles descriptor.
   */
  Style style;
  /**
   * Describes the visibility of the object.
   */
  bool visible;
  /**
   * The relative path to the image file (utf-8).
   */
  std::optional<std::string> image_file_name;
  /**
   * An array containing font style ranges. Note: Arrays are ordered.
   */
  std::optional<std::vector<TextAttr>> attr;
  /**
   * Text content of the text object. The encoding format is utf-8.
   */
  std::optional<std::string> content;
  /**
   * Specifies the text frame semantics. fixed mode strictly sets the typeset area according
   * to the frame. auto width and auto height mode expands associated bounds as needed. 0:
   * fixed 1: auto width 2: auto height
   */
  std::optional<int64_t> frame_mode;
  /**
   * If this item exists, the text is on the path and the matrix information for the text
   * should not be used.
   */
  std::optional<TextOnPath> text_on_path;
  /**
   * Text vertical alignment. 0: top 1: center 2: bottom
   */
  std::optional<int64_t> vertical_alignment;
  /**
   * Describes all child object information. The child item that appears later will be above
   * the child item that appears first.
   */
  std::optional<std::vector<Path>> child_objects;
  std::optional<std::vector<OverrideValue>> override_values;
  std::optional<double> scale;
  /**
   * The id of the referenced symbol master.
   */
  std::optional<std::string> symbol_master_id;
};

/**
 * Layers provide a way to manage all the items that make up your artwork.
 */
struct Layer
{
  std::vector<nlohmann::json> alpha_mask_by;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  Rect bounds;
  /**
   * Describes all child object information. The child item that appears later will be above
   * the child item that appears first.
   */
  std::vector<Path> child_objects;
  nlohmann::json layer_class;
  /**
   * This property of path itself is meaningless, and is represented by its style, such as
   * stroke, fill, etc.
   */
  GraphicsContextSettings context_settings;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  Rect frame;
  /**
   * Unique object identifier.
   * ID format:
   * Photoshop: numeric
   * Sketch: UUID string
   * Adobe XD: UUID string
   * Figma: string like 3:5:3
   * In Illustrator: numeric
   */
  std::string id;
  /**
   * Describes whether the object is locked.
   */
  bool is_locked;
  /**
   * Indicates whether the current object is a mask object.
   */
  bool is_mask;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::vector<double> matrix;
  /**
   * Name of object (utf-8).
   */
  std::optional<std::string> name;
  /**
   * The result of the outline mask area is the intersection of multiple outline mask object
   * results.
   */
  std::vector<nlohmann::json> outline_mask_by;
  /**
   * Object styles descriptor.
   */
  Style style;
  /**
   * Describes the visibility of the object.
   */
  bool visible;
};

/**
 * Artboard is the top-level object organization container.
 */
struct Artboard
{
  std::vector<nlohmann::json> alpha_mask_by;
  std::optional<Color> background_color;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  Rect bounds;
  nlohmann::json artboard_class;
  /**
   * This property of path itself is meaningless, and is represented by its style, such as
   * stroke, fill, etc.
   */
  GraphicsContextSettings context_settings;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  Rect frame;
  /**
   * A flag indicating whether backgroundColor should be applied as a background color of the
   * object.
   */
  bool has_background_color;
  /**
   * Unique object identifier.
   * ID format:
   * Photoshop: numeric
   * Sketch: UUID string
   * Adobe XD: UUID string
   * Figma: string like 3:5:3
   * In Illustrator: numeric
   */
  std::string id;
  /**
   * Describes whether the object is locked.
   */
  bool is_locked;
  /**
   * Indicates whether the current object is a mask object.
   */
  bool is_mask;
  std::vector<Layer> layers;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::vector<double> matrix;
  /**
   * Name of object (utf-8).
   */
  std::optional<std::string> name;
  /**
   * The result of the outline mask area is the intersection of multiple outline mask object
   * results.
   */
  std::vector<nlohmann::json> outline_mask_by;
  /**
   * Object styles descriptor.
   */
  Style style;
  /**
   * Describes the visibility of the object.
   */
  bool visible;
};

/**
 * Contains objects to define a global pattern.
 */
struct PatternLayerDef
{
  std::vector<nlohmann::json> alpha_mask_by;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  Rect bounds;
  /**
   * Describes all child object information. The child item that appears later will be above
   * the child item that appears first.
   */
  std::vector<Path> child_objects;
  nlohmann::json pattern_layer_def_class;
  /**
   * This property of path itself is meaningless, and is represented by its style, such as
   * stroke, fill, etc.
   */
  GraphicsContextSettings context_settings;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  Rect frame;
  /**
   * Unique object identifier.
   * ID format:
   * Photoshop: numeric
   * Sketch: UUID string
   * Adobe XD: UUID string
   * Figma: string like 3:5:3
   * In Illustrator: numeric
   */
  std::string id;
  /**
   * Describes whether the object is locked.
   */
  bool is_locked;
  /**
   * Indicates whether the current object is a mask object.
   */
  bool is_mask;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::vector<double> matrix;
  /**
   * Name of object (utf-8).
   */
  std::optional<std::string> name;
  /**
   * The result of the outline mask area is the intersection of multiple outline mask object
   * results.
   */
  std::vector<nlohmann::json> outline_mask_by;
  std::vector<double> pattern_bounding_box;
  /**
   * Object styles descriptor.
   */
  Style style;
  /**
   * Describes the visibility of the object.
   */
  bool visible;
};

/**
 * Defines override properties on symbol sources.
 */
struct OverrideProperty
{
  bool can_override;
  nlohmann::json override_property_class;
  std::string override_name;
};

struct PresetDictionary
{
};

/**
 * A symbol source object represents a reusable group of objects.
 */
struct SymbolMaster
{
  bool allows_overrides;
  std::vector<nlohmann::json> alpha_mask_by;
  std::optional<Color> background_color;
  /**
   * This property records the bounds information of the object before it is matrixed. For
   * example, In adobe illustrator, the top left coordinate of an image is always 0, So the
   * relationship of the bounds frame matrix is as follows: { (0, 0), (width, 0), (width,
   * -height), (0, -height) } + matrix_transformation = frame.
   */
  Rect bounds;
  /**
   * Describes all child object information. The child item that appears later will be above
   * the child item that appears first.
   */
  std::vector<Path> child_objects;
  /**
   * This property of path itself is meaningless, and is represented by its style, such as
   * stroke, fill, etc.
   */
  GraphicsContextSettings context_settings;
  /**
   * Frame information for the object. When the object is a group, its frame is the union of
   * all objects in the group, whether or not the group contains a mask. This property stores
   * information about the object after the matrix transformation.
   */
  Rect frame;
  /**
   * A flag indicating whether backgroundColor should be applied as a background color of the
   * object.
   */
  bool has_background_color;
  /**
   * Unique object identifier.
   * ID format:
   * Photoshop: numeric
   * Sketch: UUID string
   * Adobe XD: UUID string
   * Figma: string like 3:5:3
   * In Illustrator: numeric
   */
  std::string id;
  bool include_background_color_in_instance;
  /**
   * Describes whether the object is locked.
   */
  bool is_locked;
  /**
   * Indicates whether the current object is a mask object.
   */
  bool is_mask;
  /**
   * Matrix, as defined in PostScript Language Reference Manual, Second Edition.
   */
  std::vector<double> matrix;
  /**
   * Name of object (utf-8).
   */
  std::optional<std::string> name;
  /**
   * The result of the outline mask area is the intersection of multiple outline mask object
   * results.
   */
  std::vector<nlohmann::json> outline_mask_by;
  std::vector<OverrideProperty> override_properties;
  std::optional<PresetDictionary> preset_dictionary;
  /**
   * Object styles descriptor.
   */
  Style style;
  /**
   * Describes the visibility of the object.
   */
  bool visible;
};

/**
 * vgg format is a JSON-based format for describing the content of various design pieces.
 */
struct DesignDocumentModel
{
  std::vector<Artboard> artboard;
  /**
   * Specifies the path to the input design file. The encoding format is utf-8.
   */
  std::string file_name;
  /**
   * Specifies the file type. 0: photoshop 1: sketch 2: adobe xd 3: figma 4: adobe illustrator
   */
  int64_t file_type;
  std::optional<std::vector<PatternLayerDef>> pattern_layer_def;
  std::optional<std::vector<SymbolMaster>> symbol_master;
};
} // namespace VGG

namespace VGG
{
void from_json(const json& j, Color& x);
void to_json(json& j, const Color& x);

void from_json(const json& j, Rect& x);
void to_json(json& j, const Rect& x);

void from_json(const json& j, GraphicsContextSettings& x);
void to_json(json& j, const GraphicsContextSettings& x);

void from_json(const json& j, TextParagraph& x);
void to_json(json& j, const TextParagraph& x);

void from_json(const json& j, TextAttr& x);
void to_json(json& j, const TextAttr& x);

void from_json(const json& j, Data& x);
void to_json(json& j, const Data& x);

void from_json(const json& j, Sha1& x);
void to_json(json& j, const Sha1& x);

void from_json(const json& j, Reference& x);
void to_json(json& j, const Reference& x);

void from_json(const json& j, OverrideValue& x);
void to_json(json& j, const OverrideValue& x);

void from_json(const json& j, PointAttr& x);
void to_json(json& j, const PointAttr& x);

void from_json(const json& j, GradientBasicGeometry& x);
void to_json(json& j, const GradientBasicGeometry& x);

void from_json(const json& j, GradientHilight& x);
void to_json(json& j, const GradientHilight& x);

void from_json(const json& j, GradientStop& x);
void to_json(json& j, const GradientStop& x);

void from_json(const json& j, InstanceElement& x);
void to_json(json& j, const InstanceElement& x);

void from_json(const json& j, Gradient& x);
void to_json(json& j, const Gradient& x);

void from_json(const json& j, InstanceClass& x);
void to_json(json& j, const InstanceClass& x);

void from_json(const json& j, Pattern& x);
void to_json(json& j, const Pattern& x);

void from_json(const json& j, Border& x);
void to_json(json& j, const Border& x);

void from_json(const json& j, Shadow& x);
void to_json(json& j, const Shadow& x);

void from_json(const json& j, Style& x);
void to_json(json& j, const Style& x);

void from_json(const json& j, TextOnPath& x);
void to_json(json& j, const TextOnPath& x);

void from_json(const json& j, Contour& x);
void to_json(json& j, const Contour& x);

void from_json(const json& j, Subshape& x);
void to_json(json& j, const Subshape& x);

void from_json(const json& j, Shape& x);
void to_json(json& j, const Shape& x);

void from_json(const json& j, Path& x);
void to_json(json& j, const Path& x);

void from_json(const json& j, Layer& x);
void to_json(json& j, const Layer& x);

void from_json(const json& j, Artboard& x);
void to_json(json& j, const Artboard& x);

void from_json(const json& j, PatternLayerDef& x);
void to_json(json& j, const PatternLayerDef& x);

void from_json(const json& j, OverrideProperty& x);
void to_json(json& j, const OverrideProperty& x);

void from_json(const json& j, PresetDictionary& x);
void to_json(json& j, const PresetDictionary& x);

void from_json(const json& j, SymbolMaster& x);
void to_json(json& j, const SymbolMaster& x);

void from_json(const json& j, DesignDocumentModel& x);
void to_json(json& j, const DesignDocumentModel& x);

void from_json(const json& j, RefClass& x);
void to_json(json& j, const RefClass& x);
} // namespace VGG
namespace nlohmann
{
template<>
struct adl_serializer<std::variant<VGG::Reference, std::string>>
{
  static void from_json(const json& j, std::variant<VGG::Reference, std::string>& x);
  static void to_json(json& j, const std::variant<VGG::Reference, std::string>& x);
};

template<>
struct adl_serializer<std::variant<std::vector<VGG::InstanceElement>,
                                   bool,
                                   double,
                                   int64_t,
                                   std::map<std::string, json>,
                                   std::string>>
{
  static void from_json(const json& j,
                        std::variant<std::vector<VGG::InstanceElement>,
                                     bool,
                                     double,
                                     int64_t,
                                     std::map<std::string, json>,
                                     std::string>& x);
  static void to_json(json& j,
                      const std::variant<std::vector<VGG::InstanceElement>,
                                         bool,
                                         double,
                                         int64_t,
                                         std::map<std::string, json>,
                                         std::string>& x);
};
} // namespace nlohmann
namespace VGG
{
inline void from_json(const json& j, Color& x)
{
  x.alpha = j.at("alpha").get<double>();
  x.blue = j.at("blue").get<double>();
  x.color_class = get_untyped(j, "class");
  x.green = j.at("green").get<double>();
  x.red = j.at("red").get<double>();
}

inline void to_json(json& j, const Color& x)
{
  j = json::object();
  j["alpha"] = x.alpha;
  j["blue"] = x.blue;
  j["class"] = x.color_class;
  j["green"] = x.green;
  j["red"] = x.red;
}

inline void from_json(const json& j, Rect& x)
{
  x.rect_class = get_untyped(j, "class");
  x.constrain_proportions = j.at("constrainProportions").get<bool>();
  x.height = j.at("height").get<double>();
  x.width = j.at("width").get<double>();
  x.x = j.at("x").get<double>();
  x.y = j.at("y").get<double>();
}

inline void to_json(json& j, const Rect& x)
{
  j = json::object();
  j["class"] = x.rect_class;
  j["constrainProportions"] = x.constrain_proportions;
  j["height"] = x.height;
  j["width"] = x.width;
  j["x"] = x.x;
  j["y"] = x.y;
}

inline void from_json(const json& j, GraphicsContextSettings& x)
{
  x.blend_mode = j.at("blendMode").get<int64_t>();
  x.graphics_context_settings_class = get_untyped(j, "class");
  x.isolate_blending = j.at("isolateBlending").get<bool>();
  x.opacity = j.at("opacity").get<double>();
  x.transparency_knockout_group = j.at("transparencyKnockoutGroup").get<int64_t>();
}

inline void to_json(json& j, const GraphicsContextSettings& x)
{
  j = json::object();
  j["blendMode"] = x.blend_mode;
  j["class"] = x.graphics_context_settings_class;
  j["isolateBlending"] = x.isolate_blending;
  j["opacity"] = x.opacity;
  j["transparencyKnockoutGroup"] = x.transparency_knockout_group;
}

inline void from_json(const json& j, TextParagraph& x)
{
  x.text_paragraph_class = get_untyped(j, "class");
  x.paragraph_spacing = j.at("paragraphSpacing").get<double>();
}

inline void to_json(json& j, const TextParagraph& x)
{
  j = json::object();
  j["class"] = x.text_paragraph_class;
  j["paragraphSpacing"] = x.paragraph_spacing;
}

inline void from_json(const json& j, TextAttr& x)
{
  x.baseline = j.at("baseline").get<int64_t>();
  x.baseline_shift = j.at("baselineShift").get<double>();
  x.bold = get_stack_optional<bool>(j, "bold");
  x.border_color = get_stack_optional<Color>(j, "borderColor");
  x.border_size = get_stack_optional<double>(j, "borderSize");
  x.text_attr_class = get_untyped(j, "class");
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.fill_color = get_stack_optional<Color>(j, "fillColor");
  x.horizontal_alignment = j.at("horizontalAlignment").get<int64_t>();
  x.horizontal_scale = j.at("horizontalScale").get<double>();
  x.italic = get_stack_optional<bool>(j, "italic");
  x.kerning = j.at("kerning").get<bool>();
  x.length = j.at("length").get<int64_t>();
  x.letter_spacing = j.at("letterSpacing").get<double>();
  x.lettertransform = j.at("lettertransform").get<int64_t>();
  x.line_space = j.at("lineSpace").get<double>();
  x.linethrough = j.at("linethrough").get<bool>();
  x.name = j.at("name").get<std::string>();
  x.proportional_spacing = j.at("proportionalSpacing").get<double>();
  x.rotate = j.at("rotate").get<double>();
  x.size = j.at("size").get<double>();
  x.sub_family_name = get_stack_optional<std::string>(j, "subFamilyName");
  x.text_paragraph = j.at("textParagraph").get<TextParagraph>();
  x.underline = j.at("underline").get<int64_t>();
  x.vertical_scale = j.at("verticalScale").get<double>();
}

inline void to_json(json& j, const TextAttr& x)
{
  j = json::object();
  j["baseline"] = x.baseline;
  j["baselineShift"] = x.baseline_shift;
  j["bold"] = x.bold;
  j["borderColor"] = x.border_color;
  j["borderSize"] = x.border_size;
  j["class"] = x.text_attr_class;
  j["contextSettings"] = x.context_settings;
  j["fillColor"] = x.fill_color;
  j["horizontalAlignment"] = x.horizontal_alignment;
  j["horizontalScale"] = x.horizontal_scale;
  j["italic"] = x.italic;
  j["kerning"] = x.kerning;
  j["length"] = x.length;
  j["letterSpacing"] = x.letter_spacing;
  j["lettertransform"] = x.lettertransform;
  j["lineSpace"] = x.line_space;
  j["linethrough"] = x.linethrough;
  j["name"] = x.name;
  j["proportionalSpacing"] = x.proportional_spacing;
  j["rotate"] = x.rotate;
  j["size"] = x.size;
  j["subFamilyName"] = x.sub_family_name;
  j["textParagraph"] = x.text_paragraph;
  j["underline"] = x.underline;
  j["verticalScale"] = x.vertical_scale;
}

inline void from_json(const json& j, Data& x)
{
  x.data = j.at("_data").get<std::string>();
}

inline void to_json(json& j, const Data& x)
{
  j = json::object();
  j["_data"] = x.data;
}

inline void from_json(const json& j, Sha1& x)
{
  x.data = j.at("_data").get<std::string>();
}

inline void to_json(json& j, const Sha1& x)
{
  j = json::object();
  j["_data"] = x.data;
}

inline void from_json(const json& j, Reference& x)
{
  x.ref = j.at("_ref").get<std::string>();
  x.ref_class = j.at("_ref_class").get<RefClass>();
  x.reference_class = get_untyped(j, "class");
  x.data = get_stack_optional<Data>(j, "data");
  x.sha1 = get_stack_optional<Sha1>(j, "sha1");
}

inline void to_json(json& j, const Reference& x)
{
  j = json::object();
  j["_ref"] = x.ref;
  j["_ref_class"] = x.ref_class;
  j["class"] = x.reference_class;
  j["data"] = x.data;
  j["sha1"] = x.sha1;
}

inline void from_json(const json& j, OverrideValue& x)
{
  x.override_value_class = get_untyped(j, "class");
  x.do_object_id = get_stack_optional<std::string>(j, "do_objectID");
  x.override_name = j.at("overrideName").get<std::string>();
  x.value = j.at("value").get<Value>();
}

inline void to_json(json& j, const OverrideValue& x)
{
  j = json::object();
  j["class"] = x.override_value_class;
  j["do_objectID"] = x.do_object_id;
  j["overrideName"] = x.override_name;
  j["value"] = x.value;
}

inline void from_json(const json& j, PointAttr& x)
{
  x.point_attr_class = get_untyped(j, "class");
  x.points = j.at("points").get<std::vector<std::vector<double>>>();
  x.point_type = j.at("pointType").get<int64_t>();
  x.radius = get_stack_optional<double>(j, "radius");
}

inline void to_json(json& j, const PointAttr& x)
{
  j = json::object();
  j["class"] = x.point_attr_class;
  j["points"] = x.points;
  j["pointType"] = x.point_type;
  j["radius"] = x.radius;
}

inline void from_json(const json& j, GradientBasicGeometry& x)
{
  x.angle = j.at("angle").get<double>();
  x.gradient_basic_geometry_class = get_untyped(j, "class");
  x.flag = j.at("flag").get<int64_t>();
  x.length = j.at("length").get<double>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.width_ratio = j.at("widthRatio").get<double>();
  x.x_origin = j.at("xOrigin").get<double>();
  x.y_origin = j.at("yOrigin").get<double>();
}

inline void to_json(json& j, const GradientBasicGeometry& x)
{
  j = json::object();
  j["angle"] = x.angle;
  j["class"] = x.gradient_basic_geometry_class;
  j["flag"] = x.flag;
  j["length"] = x.length;
  j["matrix"] = x.matrix;
  j["widthRatio"] = x.width_ratio;
  j["xOrigin"] = x.x_origin;
  j["yOrigin"] = x.y_origin;
}

inline void from_json(const json& j, GradientHilight& x)
{
  x.angle = j.at("angle").get<double>();
  x.gradient_hilight_class = get_untyped(j, "class");
  x.length = j.at("length").get<double>();
  x.x_hilight = j.at("xHilight").get<double>();
  x.y_hilight = j.at("yHilight").get<double>();
}

inline void to_json(json& j, const GradientHilight& x)
{
  j = json::object();
  j["angle"] = x.angle;
  j["class"] = x.gradient_hilight_class;
  j["length"] = x.length;
  j["xHilight"] = x.x_hilight;
  j["yHilight"] = x.y_hilight;
}

inline void from_json(const json& j, GradientStop& x)
{
  x.gradient_stop_class = get_untyped(j, "class");
  x.color = j.at("color").get<Color>();
  x.mid_point = j.at("midPoint").get<double>();
  x.position = j.at("position").get<double>();
}

inline void to_json(json& j, const GradientStop& x)
{
  j = json::object();
  j["class"] = x.gradient_stop_class;
  j["color"] = x.color;
  j["midPoint"] = x.mid_point;
  j["position"] = x.position;
}

inline void from_json(const json& j, InstanceElement& x)
{
  x.gradient_class = get_untyped(j, "class");
  x.from = get_stack_optional<std::vector<double>>(j, "from");
  x.invert = j.at("invert").get<bool>();
  x.rotation = get_stack_optional<double>(j, "rotation");
  x.stops = j.at("stops").get<std::vector<GradientStop>>();
  x.to = get_stack_optional<std::vector<double>>(j, "to");
  x.elipse_length = get_stack_optional<double>(j, "elipseLength");
  x.geometry = get_stack_optional<GradientBasicGeometry>(j, "geometry");
  x.gradient_type = get_stack_optional<int64_t>(j, "gradientType");
  x.hilight = get_stack_optional<GradientHilight>(j, "hilight");
  x.overall_matrix = get_stack_optional<std::vector<double>>(j, "overallMatrix");
  x.perpendicular_matrix =
    get_stack_optional<std::vector<nlohmann::json>>(j, "perpendicularMatrix");
}

inline void to_json(json& j, const InstanceElement& x)
{
  j = json::object();
  j["class"] = x.gradient_class;
  j["from"] = x.from;
  j["invert"] = x.invert;
  j["rotation"] = x.rotation;
  j["stops"] = x.stops;
  j["to"] = x.to;
  j["elipseLength"] = x.elipse_length;
  j["geometry"] = x.geometry;
  j["gradientType"] = x.gradient_type;
  j["hilight"] = x.hilight;
  j["overallMatrix"] = x.overall_matrix;
  j["perpendicularMatrix"] = x.perpendicular_matrix;
}

inline void from_json(const json& j, Gradient& x)
{
  x.gradient_class = get_untyped(j, "class");
  x.instance = get_stack_optional<std::variant<std::vector<InstanceElement>,
                                               bool,
                                               double,
                                               int64_t,
                                               std::map<std::string, nlohmann::json>,
                                               std::string>>(j, "instance");
}

inline void to_json(json& j, const Gradient& x)
{
  j = json::object();
  j["class"] = x.gradient_class;
  j["instance"] = x.instance;
}

inline void from_json(const json& j, InstanceClass& x)
{
  x.pattern_class = get_untyped(j, "class");
  x.fill_type = get_stack_optional<int64_t>(j, "fillType");
  x.image_file_name = get_stack_optional<std::string>(j, "imageFileName");
  x.image_tile_mirrored = get_stack_optional<bool>(j, "imageTileMirrored");
  x.image_tile_scale = get_stack_optional<double>(j, "imageTileScale");
  x.angle = get_stack_optional<double>(j, "angle");
  x.matrix = get_stack_optional<std::vector<double>>(j, "matrix");
  x.offset = get_stack_optional<std::vector<double>>(j, "offset");
  x.r = get_stack_optional<double>(j, "r");
  x.ref_layer_name = get_stack_optional<std::string>(j, "refLayerName");
  x.reflection = get_stack_optional<bool>(j, "reflection");
  x.scale = get_stack_optional<std::vector<double>>(j, "scale");
  x.shear = get_stack_optional<double>(j, "shear");
  x.shear_axis = get_stack_optional<double>(j, "shearAxis");
}

inline void to_json(json& j, const InstanceClass& x)
{
  j = json::object();
  j["class"] = x.pattern_class;
  j["fillType"] = x.fill_type;
  j["imageFileName"] = x.image_file_name;
  j["imageTileMirrored"] = x.image_tile_mirrored;
  j["imageTileScale"] = x.image_tile_scale;
  j["angle"] = x.angle;
  j["matrix"] = x.matrix;
  j["offset"] = x.offset;
  j["r"] = x.r;
  j["refLayerName"] = x.ref_layer_name;
  j["reflection"] = x.reflection;
  j["scale"] = x.scale;
  j["shear"] = x.shear;
  j["shearAxis"] = x.shear_axis;
}

inline void from_json(const json& j, Pattern& x)
{
  x.pattern_class = get_untyped(j, "class");
  x.instance = j.at("instance").get<InstanceClass>();
}

inline void to_json(json& j, const Pattern& x)
{
  j = json::object();
  j["class"] = x.pattern_class;
  j["instance"] = x.instance;
}

inline void from_json(const json& j, Border& x)
{
  x.border_class = get_untyped(j, "class");
  x.color = get_stack_optional<Color>(j, "color");
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.dashed_offset = j.at("dashedOffset").get<double>();
  x.dashed_pattern = j.at("dashedPattern").get<std::vector<double>>();
  x.fill_type = j.at("fillType").get<int64_t>();
  x.flat = j.at("flat").get<double>();
  x.gradient = get_stack_optional<Gradient>(j, "gradient");
  x.is_enabled = j.at("isEnabled").get<bool>();
  x.line_cap_style = j.at("lineCapStyle").get<int64_t>();
  x.line_join_style = j.at("lineJoinStyle").get<int64_t>();
  x.miter_limit = j.at("miterLimit").get<double>();
  x.pattern = get_stack_optional<Pattern>(j, "pattern");
  x.position = j.at("position").get<int64_t>();
  x.style = j.at("style").get<int64_t>();
  x.thickness = j.at("thickness").get<double>();
}

inline void to_json(json& j, const Border& x)
{
  j = json::object();
  j["class"] = x.border_class;
  j["color"] = x.color;
  j["contextSettings"] = x.context_settings;
  j["dashedOffset"] = x.dashed_offset;
  j["dashedPattern"] = x.dashed_pattern;
  j["fillType"] = x.fill_type;
  j["flat"] = x.flat;
  j["gradient"] = x.gradient;
  j["isEnabled"] = x.is_enabled;
  j["lineCapStyle"] = x.line_cap_style;
  j["lineJoinStyle"] = x.line_join_style;
  j["miterLimit"] = x.miter_limit;
  j["pattern"] = x.pattern;
  j["position"] = x.position;
  j["style"] = x.style;
  j["thickness"] = x.thickness;
}

inline void from_json(const json& j, Shadow& x)
{
  x.blur = j.at("blur").get<double>();
  x.shadow_class = get_untyped(j, "class");
  x.color = j.at("color").get<Color>();
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.inner = j.at("inner").get<bool>();
  x.is_enabled = j.at("isEnabled").get<bool>();
  x.offset_x = j.at("offsetX").get<double>();
  x.offset_y = j.at("offsetY").get<double>();
  x.spread = j.at("spread").get<double>();
}

inline void to_json(json& j, const Shadow& x)
{
  j = json::object();
  j["blur"] = x.blur;
  j["class"] = x.shadow_class;
  j["color"] = x.color;
  j["contextSettings"] = x.context_settings;
  j["inner"] = x.inner;
  j["isEnabled"] = x.is_enabled;
  j["offsetX"] = x.offset_x;
  j["offsetY"] = x.offset_y;
  j["spread"] = x.spread;
}

inline void from_json(const json& j, Style& x)
{
  x.blurs = j.at("blurs").get<std::vector<nlohmann::json>>();
  x.borders = j.at("borders").get<std::vector<Border>>();
  x.style_class = get_untyped(j, "class");
  x.fills = j.at("fills").get<std::vector<nlohmann::json>>();
  x.shadows = j.at("shadows").get<std::vector<Shadow>>();
}

inline void to_json(json& j, const Style& x)
{
  j = json::object();
  j["blurs"] = x.blurs;
  j["borders"] = x.borders;
  j["class"] = x.style_class;
  j["fills"] = x.fills;
  j["shadows"] = x.shadows;
}

inline void from_json(const json& j, TextOnPath& x)
{
  x.text_on_path_class = get_untyped(j, "class");
  x.path_id = j.at("pathId").get<std::string>();
  x.start_offset = j.at("startOffset").get<double>();
}

inline void to_json(json& j, const TextOnPath& x)
{
  j = json::object();
  j["class"] = x.text_on_path_class;
  j["pathId"] = x.path_id;
  j["startOffset"] = x.start_offset;
}

inline void from_json(const json& j, Contour& x)
{
  x.contour_class = get_untyped(j, "class");
  x.closed = get_stack_optional<bool>(j, "closed");
  x.points = get_stack_optional<std::vector<PointAttr>>(j, "points");
  x.alpha_mask_by = get_stack_optional<std::vector<nlohmann::json>>(j, "alphaMaskBy");
  x.attr = get_stack_optional<std::vector<TextAttr>>(j, "attr");
  x.bounds = get_stack_optional<Rect>(j, "bounds");
  x.content = get_stack_optional<std::string>(j, "content");
  x.context_settings = get_stack_optional<GraphicsContextSettings>(j, "contextSettings");
  x.frame = get_stack_optional<Rect>(j, "frame");
  x.frame_mode = get_stack_optional<int64_t>(j, "frameMode");
  x.id = get_stack_optional<std::string>(j, "id");
  x.is_locked = get_stack_optional<bool>(j, "isLocked");
  x.is_mask = get_stack_optional<bool>(j, "isMask");
  x.matrix = get_stack_optional<std::vector<double>>(j, "matrix");
  x.name = get_stack_optional<std::string>(j, "name");
  x.outline_mask_by = get_stack_optional<std::vector<nlohmann::json>>(j, "outlineMaskBy");
  x.style = get_stack_optional<Style>(j, "style");
  x.text_on_path = get_stack_optional<TextOnPath>(j, "textOnPath");
  x.vertical_alignment = get_stack_optional<int64_t>(j, "verticalAlignment");
  x.visible = get_stack_optional<bool>(j, "visible");
  x.image_file_name = get_stack_optional<std::string>(j, "imageFileName");
  x.override_values = get_stack_optional<std::vector<OverrideValue>>(j, "overrideValues");
  x.scale = get_stack_optional<double>(j, "scale");
  x.symbol_master_id = get_stack_optional<std::string>(j, "symbolMasterId");
}

inline void to_json(json& j, const Contour& x)
{
  j = json::object();
  j["class"] = x.contour_class;
  j["closed"] = x.closed;
  j["points"] = x.points;
  j["alphaMaskBy"] = x.alpha_mask_by;
  j["attr"] = x.attr;
  j["bounds"] = x.bounds;
  j["content"] = x.content;
  j["contextSettings"] = x.context_settings;
  j["frame"] = x.frame;
  j["frameMode"] = x.frame_mode;
  j["id"] = x.id;
  j["isLocked"] = x.is_locked;
  j["isMask"] = x.is_mask;
  j["matrix"] = x.matrix;
  j["name"] = x.name;
  j["outlineMaskBy"] = x.outline_mask_by;
  j["style"] = x.style;
  j["textOnPath"] = x.text_on_path;
  j["verticalAlignment"] = x.vertical_alignment;
  j["visible"] = x.visible;
  j["imageFileName"] = x.image_file_name;
  j["overrideValues"] = x.override_values;
  j["scale"] = x.scale;
  j["symbolMasterId"] = x.symbol_master_id;
}

inline void from_json(const json& j, Subshape& x)
{
  x.boolean_operation = j.at("booleanOperation").get<int64_t>();
  x.bounds = j.at("bounds").get<Rect>();
  x.subshape_class = get_untyped(j, "class");
  x.frame = j.at("frame").get<Rect>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.sub_geometry = j.at("subGeometry").get<Contour>();
}

inline void to_json(json& j, const Subshape& x)
{
  j = json::object();
  j["booleanOperation"] = x.boolean_operation;
  j["bounds"] = x.bounds;
  j["class"] = x.subshape_class;
  j["frame"] = x.frame;
  j["matrix"] = x.matrix;
  j["subGeometry"] = x.sub_geometry;
}

inline void from_json(const json& j, Shape& x)
{
  x.shape_class = get_untyped(j, "class");
  x.subshapes = j.at("subshapes").get<std::vector<Subshape>>();
  x.winding_rule = j.at("windingRule").get<int64_t>();
}

inline void to_json(json& j, const Shape& x)
{
  j = json::object();
  j["class"] = x.shape_class;
  j["subshapes"] = x.subshapes;
  j["windingRule"] = x.winding_rule;
}

inline void from_json(const json& j, Path& x)
{
  x.alpha_mask_by = j.at("alphaMaskBy").get<std::vector<nlohmann::json>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.path_class = get_untyped(j, "class");
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.frame = j.at("frame").get<Rect>();
  x.id = j.at("id").get<std::string>();
  x.is_locked = j.at("isLocked").get<bool>();
  x.is_mask = j.at("isMask").get<bool>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outline_mask_by = j.at("outlineMaskBy").get<std::vector<nlohmann::json>>();
  x.shape = get_stack_optional<Shape>(j, "shape");
  x.style = j.at("style").get<Style>();
  x.visible = j.at("visible").get<bool>();
  x.image_file_name = get_stack_optional<std::string>(j, "imageFileName");
  x.attr = get_stack_optional<std::vector<TextAttr>>(j, "attr");
  x.content = get_stack_optional<std::string>(j, "content");
  x.frame_mode = get_stack_optional<int64_t>(j, "frameMode");
  x.text_on_path = get_stack_optional<TextOnPath>(j, "textOnPath");
  x.vertical_alignment = get_stack_optional<int64_t>(j, "verticalAlignment");
  x.child_objects = get_stack_optional<std::vector<Path>>(j, "childObjects");
  x.override_values = get_stack_optional<std::vector<OverrideValue>>(j, "overrideValues");
  x.scale = get_stack_optional<double>(j, "scale");
  x.symbol_master_id = get_stack_optional<std::string>(j, "symbolMasterId");
}

inline void to_json(json& j, const Path& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alpha_mask_by;
  j["bounds"] = x.bounds;
  j["class"] = x.path_class;
  j["contextSettings"] = x.context_settings;
  j["frame"] = x.frame;
  j["id"] = x.id;
  j["isLocked"] = x.is_locked;
  j["isMask"] = x.is_mask;
  j["matrix"] = x.matrix;
  j["name"] = x.name;
  j["outlineMaskBy"] = x.outline_mask_by;
  j["shape"] = x.shape;
  j["style"] = x.style;
  j["visible"] = x.visible;
  j["imageFileName"] = x.image_file_name;
  j["attr"] = x.attr;
  j["content"] = x.content;
  j["frameMode"] = x.frame_mode;
  j["textOnPath"] = x.text_on_path;
  j["verticalAlignment"] = x.vertical_alignment;
  j["childObjects"] = x.child_objects;
  j["overrideValues"] = x.override_values;
  j["scale"] = x.scale;
  j["symbolMasterId"] = x.symbol_master_id;
}

inline void from_json(const json& j, Layer& x)
{
  x.alpha_mask_by = j.at("alphaMaskBy").get<std::vector<nlohmann::json>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.child_objects = j.at("childObjects").get<std::vector<Path>>();
  x.layer_class = get_untyped(j, "class");
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.frame = j.at("frame").get<Rect>();
  x.id = j.at("id").get<std::string>();
  x.is_locked = j.at("isLocked").get<bool>();
  x.is_mask = j.at("isMask").get<bool>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outline_mask_by = j.at("outlineMaskBy").get<std::vector<nlohmann::json>>();
  x.style = j.at("style").get<Style>();
  x.visible = j.at("visible").get<bool>();
}

inline void to_json(json& j, const Layer& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alpha_mask_by;
  j["bounds"] = x.bounds;
  j["childObjects"] = x.child_objects;
  j["class"] = x.layer_class;
  j["contextSettings"] = x.context_settings;
  j["frame"] = x.frame;
  j["id"] = x.id;
  j["isLocked"] = x.is_locked;
  j["isMask"] = x.is_mask;
  j["matrix"] = x.matrix;
  j["name"] = x.name;
  j["outlineMaskBy"] = x.outline_mask_by;
  j["style"] = x.style;
  j["visible"] = x.visible;
}

inline void from_json(const json& j, Artboard& x)
{
  x.alpha_mask_by = j.at("alphaMaskBy").get<std::vector<nlohmann::json>>();
  x.background_color = get_stack_optional<Color>(j, "backgroundColor");
  x.bounds = j.at("bounds").get<Rect>();
  x.artboard_class = get_untyped(j, "class");
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.frame = j.at("frame").get<Rect>();
  x.has_background_color = j.at("hasBackgroundColor").get<bool>();
  x.id = j.at("id").get<std::string>();
  x.is_locked = j.at("isLocked").get<bool>();
  x.is_mask = j.at("isMask").get<bool>();
  x.layers = j.at("layers").get<std::vector<Layer>>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outline_mask_by = j.at("outlineMaskBy").get<std::vector<nlohmann::json>>();
  x.style = j.at("style").get<Style>();
  x.visible = j.at("visible").get<bool>();
}

inline void to_json(json& j, const Artboard& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alpha_mask_by;
  j["backgroundColor"] = x.background_color;
  j["bounds"] = x.bounds;
  j["class"] = x.artboard_class;
  j["contextSettings"] = x.context_settings;
  j["frame"] = x.frame;
  j["hasBackgroundColor"] = x.has_background_color;
  j["id"] = x.id;
  j["isLocked"] = x.is_locked;
  j["isMask"] = x.is_mask;
  j["layers"] = x.layers;
  j["matrix"] = x.matrix;
  j["name"] = x.name;
  j["outlineMaskBy"] = x.outline_mask_by;
  j["style"] = x.style;
  j["visible"] = x.visible;
}

inline void from_json(const json& j, PatternLayerDef& x)
{
  x.alpha_mask_by = j.at("alphaMaskBy").get<std::vector<nlohmann::json>>();
  x.bounds = j.at("bounds").get<Rect>();
  x.child_objects = j.at("childObjects").get<std::vector<Path>>();
  x.pattern_layer_def_class = get_untyped(j, "class");
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.frame = j.at("frame").get<Rect>();
  x.id = j.at("id").get<std::string>();
  x.is_locked = j.at("isLocked").get<bool>();
  x.is_mask = j.at("isMask").get<bool>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outline_mask_by = j.at("outlineMaskBy").get<std::vector<nlohmann::json>>();
  x.pattern_bounding_box = j.at("pattern_bounding_box").get<std::vector<double>>();
  x.style = j.at("style").get<Style>();
  x.visible = j.at("visible").get<bool>();
}

inline void to_json(json& j, const PatternLayerDef& x)
{
  j = json::object();
  j["alphaMaskBy"] = x.alpha_mask_by;
  j["bounds"] = x.bounds;
  j["childObjects"] = x.child_objects;
  j["class"] = x.pattern_layer_def_class;
  j["contextSettings"] = x.context_settings;
  j["frame"] = x.frame;
  j["id"] = x.id;
  j["isLocked"] = x.is_locked;
  j["isMask"] = x.is_mask;
  j["matrix"] = x.matrix;
  j["name"] = x.name;
  j["outlineMaskBy"] = x.outline_mask_by;
  j["pattern_bounding_box"] = x.pattern_bounding_box;
  j["style"] = x.style;
  j["visible"] = x.visible;
}

inline void from_json(const json& j, OverrideProperty& x)
{
  x.can_override = j.at("canOverride").get<bool>();
  x.override_property_class = get_untyped(j, "class");
  x.override_name = j.at("overrideName").get<std::string>();
}

inline void to_json(json& j, const OverrideProperty& x)
{
  j = json::object();
  j["canOverride"] = x.can_override;
  j["class"] = x.override_property_class;
  j["overrideName"] = x.override_name;
}

inline void from_json(const json& j, PresetDictionary& x)
{
}

inline void to_json(json& j, const PresetDictionary& x)
{
  j = json::object();
}

inline void from_json(const json& j, SymbolMaster& x)
{
  x.allows_overrides = j.at("allowsOverrides").get<bool>();
  x.alpha_mask_by = j.at("alphaMaskBy").get<std::vector<nlohmann::json>>();
  x.background_color = get_stack_optional<Color>(j, "backgroundColor");
  x.bounds = j.at("bounds").get<Rect>();
  x.child_objects = j.at("childObjects").get<std::vector<Path>>();
  x.context_settings = j.at("contextSettings").get<GraphicsContextSettings>();
  x.frame = j.at("frame").get<Rect>();
  x.has_background_color = j.at("hasBackgroundColor").get<bool>();
  x.id = j.at("id").get<std::string>();
  x.include_background_color_in_instance = j.at("includeBackgroundColorInInstance").get<bool>();
  x.is_locked = j.at("isLocked").get<bool>();
  x.is_mask = j.at("isMask").get<bool>();
  x.matrix = j.at("matrix").get<std::vector<double>>();
  x.name = get_stack_optional<std::string>(j, "name");
  x.outline_mask_by = j.at("outlineMaskBy").get<std::vector<nlohmann::json>>();
  x.override_properties = j.at("overrideProperties").get<std::vector<OverrideProperty>>();
  x.preset_dictionary = get_stack_optional<PresetDictionary>(j, "presetDictionary");
  x.style = j.at("style").get<Style>();
  x.visible = j.at("visible").get<bool>();
}

inline void to_json(json& j, const SymbolMaster& x)
{
  j = json::object();
  j["allowsOverrides"] = x.allows_overrides;
  j["alphaMaskBy"] = x.alpha_mask_by;
  j["backgroundColor"] = x.background_color;
  j["bounds"] = x.bounds;
  j["childObjects"] = x.child_objects;
  j["contextSettings"] = x.context_settings;
  j["frame"] = x.frame;
  j["hasBackgroundColor"] = x.has_background_color;
  j["id"] = x.id;
  j["includeBackgroundColorInInstance"] = x.include_background_color_in_instance;
  j["isLocked"] = x.is_locked;
  j["isMask"] = x.is_mask;
  j["matrix"] = x.matrix;
  j["name"] = x.name;
  j["outlineMaskBy"] = x.outline_mask_by;
  j["overrideProperties"] = x.override_properties;
  j["presetDictionary"] = x.preset_dictionary;
  j["style"] = x.style;
  j["visible"] = x.visible;
}

inline void from_json(const json& j, DesignDocumentModel& x)
{
  x.artboard = j.at("artboard").get<std::vector<Artboard>>();
  x.file_name = j.at("fileName").get<std::string>();
  x.file_type = j.at("fileType").get<int64_t>();
  x.pattern_layer_def = get_stack_optional<std::vector<PatternLayerDef>>(j, "patternLayerDef");
  x.symbol_master = get_stack_optional<std::vector<SymbolMaster>>(j, "symbolMaster");
}

inline void to_json(json& j, const DesignDocumentModel& x)
{
  j = json::object();
  j["artboard"] = x.artboard;
  j["fileName"] = x.file_name;
  j["fileType"] = x.file_type;
  j["patternLayerDef"] = x.pattern_layer_def;
  j["symbolMaster"] = x.symbol_master;
}

inline void from_json(const json& j, RefClass& x)
{
  if (j == "MSFontData")
    x = RefClass::MS_FONT_DATA;
  else if (j == "MSImageData")
    x = RefClass::MS_IMAGE_DATA;
  else if (j == "MSImmutablePage")
    x = RefClass::MS_IMMUTABLE_PAGE;
  else if (j == "MSPatch")
    x = RefClass::MS_PATCH;
  else
  {
    throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void to_json(json& j, const RefClass& x)
{
  switch (x)
  {
    case RefClass::MS_FONT_DATA:
      j = "MSFontData";
      break;
    case RefClass::MS_IMAGE_DATA:
      j = "MSImageData";
      break;
    case RefClass::MS_IMMUTABLE_PAGE:
      j = "MSImmutablePage";
      break;
    case RefClass::MS_PATCH:
      j = "MSPatch";
      break;
    default:
      throw std::runtime_error("This should not happen");
  }
}
} // namespace VGG
namespace nlohmann
{
inline void adl_serializer<std::variant<VGG::Reference, std::string>>::from_json(
  const json& j,
  std::variant<VGG::Reference, std::string>& x)
{
  if (j.is_string())
    x = j.get<std::string>();
  else if (j.is_object())
    x = j.get<VGG::Reference>();
  else
    throw std::runtime_error("Could not deserialise!");
}

inline void adl_serializer<std::variant<VGG::Reference, std::string>>::to_json(
  json& j,
  const std::variant<VGG::Reference, std::string>& x)
{
  switch (x.index())
  {
    case 0:
      j = std::get<VGG::Reference>(x);
      break;
    case 1:
      j = std::get<std::string>(x);
      break;
    default:
      throw std::runtime_error("Input JSON does not conform to schema!");
  }
}

inline void adl_serializer<
  std::variant<std::vector<VGG::InstanceElement>,
               bool,
               double,
               int64_t,
               std::map<std::string, json>,
               std::string>>::from_json(const json& j,
                                        std::variant<std::vector<VGG::InstanceElement>,
                                                     bool,
                                                     double,
                                                     int64_t,
                                                     std::map<std::string, json>,
                                                     std::string>& x)
{
  if (j.is_boolean())
    x = j.get<bool>();
  else if (j.is_number_integer())
    x = j.get<int64_t>();
  else if (j.is_number())
    x = j.get<double>();
  else if (j.is_string())
    x = j.get<std::string>();
  else if (j.is_object())
    x = j.get<std::map<std::string, json>>();
  else if (j.is_array())
    x = j.get<std::vector<VGG::InstanceElement>>();
  else
    throw std::runtime_error("Could not deserialise!");
}

inline void adl_serializer<
  std::variant<std::vector<VGG::InstanceElement>,
               bool,
               double,
               int64_t,
               std::map<std::string, json>,
               std::string>>::to_json(json& j,
                                      const std::variant<std::vector<VGG::InstanceElement>,
                                                         bool,
                                                         double,
                                                         int64_t,
                                                         std::map<std::string, json>,
                                                         std::string>& x)
{
  switch (x.index())
  {
    case 0:
      j = std::get<std::vector<VGG::InstanceElement>>(x);
      break;
    case 1:
      j = std::get<bool>(x);
      break;
    case 2:
      j = std::get<double>(x);
      break;
    case 3:
      j = std::get<int64_t>(x);
      break;
    case 4:
      j = std::get<std::map<std::string, json>>(x);
      break;
    case 5:
      j = std::get<std::string>(x);
      break;
    default:
      throw std::runtime_error("Input JSON does not conform to schema!");
  }
}
} // namespace nlohmann
