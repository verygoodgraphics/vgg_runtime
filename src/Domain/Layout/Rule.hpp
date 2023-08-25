#pragma once

#include "nlohmann/json.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace VGG
{
namespace Layout
{
namespace Internal
{
namespace Rule
{

struct Length
{
  enum class Types
  {
    px = 1,
    percent = 2,
    fit_content = 4
  };

  Types types;
  double value;
};
struct Width
{
  Length value;
};
struct Height
{
  Length value;
};
struct MaxHeight
{
  Length value;
};
struct MaxWidth
{
  Length value;
};
struct MinHeight
{
  Length value;
};
struct MinWidth
{
  Length value;
};

struct Left
{
  double value;
};
struct Right
{
  double value;
};
struct Top
{
  double value;
};
struct Bottom
{
  double value;
};

struct GridItemPos
{
  enum class Strategy
  {
    Auto = 1,
    Fix = 2
  };

  Strategy strategy;
  int64_t column_id;
  int64_t row_id;
};

struct Position
{
  enum class Types
  {
    Relative = 1,
    Absolute = 2,
    Fixed = 3,
    Sticky = 4
  };

  Types value;
};

struct ColumnWidth
{
  enum class Strategy
  {
    Min = 1,
    Fix = 2
  };
  Strategy strategy;
  double width_value;
};

struct ExpandStrategy
{
  enum class Strategy
  {
    Auto = 1,
    Fix_column = 2
  };
  Strategy strategy;
  int64_t min_row;
  int64_t column_count;
};

struct RowHeight
{
  enum class Strategy
  {
    FillContainer = 1,
    FillContent = 2,
    Fixed = 3
  };
  Strategy strategy;
  double fixed_value;
};

enum class AlignStyle
{
  Start = 1,
  Center = 2,
  End = 3
};

struct FlexboxItem
{
  Position position;
  double flex_grow;

  std::optional<Top> top;
  std::optional<Right> right;
  std::optional<Bottom> bottom;
  std::optional<Left> left;
};
struct GridItem
{
  GridItemPos item_pos;
  int64_t row_span;
  int64_t column_span;
  Position position;
  AlignStyle row_align;
  AlignStyle column_align;

  std::optional<Top> top;
  std::optional<Right> right;
  std::optional<Bottom> bottom;
  std::optional<Left> left;
};

struct Padding
{
  double top;
  double right;
  double bottom;
  double left;
};

struct FlexboxLayout
{
  enum class Direction
  {
    Horizontal = 1,
    Vertical = 2
  };
  enum class JustifyContent
  {
    Start = 1,
    Center = 2,
    End = 3,
    SpaceBetween = 4,
    SpaceAround = 5,
    SpaceEvenly = 6
  };
  enum class Wrap
  {
    NoWrap = 1,
    Wrap = 2
  };
  Direction direction;
  JustifyContent justify_content;
  AlignStyle align_items;
  AlignStyle align_content;
  Wrap wrap;
  double row_gap;
  double column_gap;
  Padding padding;
};

struct GridLayout
{
  ExpandStrategy expand_strategy;
  ColumnWidth column_width;
  RowHeight row_height;
  int base_height;
  int column_gap;
  int row_gap;
  int grid_auto_flow;
  Padding padding;
  AlignStyle cell_align;
};

struct Rule
{
  std::string id;

  std::variant<std::monostate, FlexboxLayout, GridLayout> layout;
  std::variant<std::monostate, FlexboxItem, GridItem> item_in_layout;

  Width width;
  std::optional<MaxWidth> max_width;
  std::optional<MinWidth> min_width;

  Height height;
  std::optional<MaxHeight> max_height;
  std::optional<MinHeight> min_height;

  std::optional<double> aspect_ratio;

  auto getFlexConatinerRule()
  {
    return std::get_if<FlexboxLayout>(&layout);
  }
  auto getGridConatinerRule()
  {
    return std::get_if<GridLayout>(&layout);
  }
  auto getFlexItemRule()
  {
    return std::get_if<FlexboxItem>(&item_in_layout);
  }
  auto getGridItemRule()
  {
    return std::get_if<GridItem>(&item_in_layout);
  }

  bool isFlexConatiner()
  {
    return getFlexConatinerRule();
  }
  bool isGridConatiner()
  {
    return getGridConatinerRule();
  }
  bool isFlexItem()
  {
    return getFlexItemRule();
  }
  bool isGridItem()
  {
    return getGridItemRule();
  }
};

// NOLINTBEGIN
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Length, types, value);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Bottom, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ColumnWidth, strategy, width_value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ExpandStrategy, strategy, min_row, column_count);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(GridItemPos, strategy, column_id, row_id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Height, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Left, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MaxHeight, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MaxWidth, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MinHeight, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MinWidth, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Position, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Right, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RowHeight, strategy, fixed_value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Top, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Width, value);

void from_json(const nlohmann::json& json, FlexboxItem& obj);
void from_json(const nlohmann::json& json, GridItem& obj);
void from_json(const nlohmann::json& json, Padding& obj);
void to_json(nlohmann::json& json, const FlexboxItem& obj);
void to_json(nlohmann::json& json, const GridItem& obj);
void to_json(nlohmann::json& json, const Padding& obj);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FlexboxLayout,
                                                direction,
                                                justify_content,
                                                align_items,
                                                align_content,
                                                wrap,
                                                row_gap,
                                                column_gap,
                                                padding);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(GridLayout,
                                                expand_strategy,
                                                column_width,
                                                row_height,
                                                base_height,
                                                column_gap,
                                                row_gap,
                                                grid_auto_flow,
                                                padding,
                                                cell_align);

void from_json(const nlohmann::json& json, Rule& obj);
void to_json(nlohmann::json& json, const Rule& obj);

// NOLINTEND

} // namespace Rule
} // namespace Internal
} // namespace Layout
} // namespace VGG
