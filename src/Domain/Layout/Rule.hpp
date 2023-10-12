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
  enum class ETypes
  {
    PX = 1,
    PERCENT = 2,
    FIT_CONTENT = 4
  };

  ETypes types;
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
  enum class EStrategy
  {
    AUTO = 1,
    FIX = 2
  };

  EStrategy strategy;
  int64_t column_id; // NOLINT
  int64_t row_id;    // NOLINT
};

struct Position
{
  enum class ETypes
  {
    RELATIVE = 1,
    ABSOLUTE = 2,
    FIXED = 3,
    STICKY = 4
  };

  ETypes value;
};

struct ColumnWidth
{
  enum class EStrategy
  {
    MIN = 1,
    FIX = 2
  };
  EStrategy strategy;
  double width_value; // NOLINT
};

struct ExpandStrategy
{
  enum class EStrategy
  {
    AUTO = 1,
    FIX_COLUMN = 2
  };
  EStrategy strategy;
  int64_t min_row;      // NOLINT
  int64_t column_count; // NOLINT
};

struct RowHeight
{
  enum class EStrategy
  {
    FILL_CONTAINER = 1,
    FILL_CONTENT = 2,
    FIXED = 3
  };
  EStrategy strategy;
  double fixed_value; // NOLINT
};

enum class EAlignStyle
{
  START = 1,
  CENTER = 2,
  END = 3
};

struct FlexboxItem
{
  Position position;
  double flex_grow_shrink; // NOLINT

  std::optional<Top> top;
  std::optional<Right> right;
  std::optional<Bottom> bottom;
  std::optional<Left> left;
};
struct GridItem
{
  // NOLINTBEGIN
  GridItemPos item_pos;
  int64_t row_span;
  int64_t column_span;
  Position position;
  EAlignStyle row_align;
  EAlignStyle column_align;

  std::optional<Top> top;
  std::optional<Right> right;
  std::optional<Bottom> bottom;
  std::optional<Left> left;
  // NOLINTEND
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
  enum class EDirection
  {
    HORIZONTAL = 1,
    VERTICAL = 2
  };
  enum class EJustifyContent
  {
    START = 1,
    CENTER = 2,
    END = 3,
    SPACE_BETWEEN = 4,
    SPACE_AROUND = 5,
    SPACE_EVENLY = 6
  };
  enum class EWrap
  {
    NO_WRAP = 1,
    WRAP = 2
  };
  // NOLINTBEGIN
  EDirection direction;
  EJustifyContent justify_content;
  EAlignStyle align_items;
  EAlignStyle align_content;
  EWrap wrap;
  double row_gap;
  double column_gap;
  Padding padding;
  bool z_order;
  // NOLINTEND
};

struct GridLayout
{
  // NOLINTBEGIN
  ExpandStrategy expand_strategy;
  ColumnWidth column_width;
  RowHeight row_height;
  int base_height;
  int column_gap;
  int row_gap;
  int grid_auto_flow;
  Padding padding;
  EAlignStyle cell_align;
  // NOLINTEND
};

struct Rule
{
  // NOLINTBEGIN
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
  // NOLINTEND

  auto getFlexContainerRule()
  {
    return std::get_if<FlexboxLayout>(&layout);
  }
  auto getGridContainerRule()
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

  bool isFlexContainer()
  {
    return getFlexContainerRule();
  }
  bool isGridContainer()
  {
    return getGridContainerRule();
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
                                                padding,
                                                z_order);
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
