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

  bool is100Percent();
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
  int64_t   columnId;
  int64_t   rowId;
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
  double    widthValue;
};

struct ExpandStrategy
{
  enum class EStrategy
  {
    AUTO = 1,
    FIX_COLUMN = 2
  };
  EStrategy strategy;
  int64_t   minRow;
  int64_t   columnCount;
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
  double    fixedValue;
};

enum class EAlignStyle
{
  START = 1,
  CENTER = 2,
  END = 3,
  SPACE_BETWEEN = 4,
  SPACE_AROUND = 5,
  SPACE_EVENLY = 6
};

struct FlexboxItem
{
  Position position;
  double   flexBasis;

  std::optional<Top>    top;
  std::optional<Right>  right;
  std::optional<Bottom> bottom;
  std::optional<Left>   left;
};
struct GridItem
{
  GridItemPos itemPos;
  int64_t     rowSpan;
  int64_t     columnSpan;
  Position    position;
  EAlignStyle rowAlign;
  EAlignStyle columnAlign;

  std::optional<Top>    top;
  std::optional<Right>  right;
  std::optional<Bottom> bottom;
  std::optional<Left>   left;
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
  EDirection      direction;
  EJustifyContent justifyContent;
  EAlignStyle     alignItems;
  EAlignStyle     alignContent;
  EWrap           wrap;
  double          rowGap;
  double          columnGap;
  Padding         padding;
  bool            zOrder;
};

struct GridLayout
{
  ExpandStrategy expandStrategy;
  ColumnWidth    columnWidth;
  RowHeight      rowHeight;
  int            baseHeight;
  int            columnGap;
  int            rowGap;
  int            gridAutoFlow;
  Padding        padding;
  EAlignStyle    cellAlign;
};

struct Rule
{
  std::string id;

  std::variant<std::monostate, FlexboxLayout, GridLayout> layout;
  std::variant<std::monostate, FlexboxItem, GridItem>     itemInLayout;

  Width                   width;
  std::optional<MaxWidth> maxWidth;
  std::optional<MinWidth> minWidth;

  Height                   height;
  std::optional<MaxHeight> maxHeight;
  std::optional<MinHeight> minHeight;

  std::optional<double> aspectRatio;

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
    return std::get_if<FlexboxItem>(&itemInLayout);
  }
  auto getGridItemRule()
  {
    return std::get_if<GridItem>(&itemInLayout);
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Length, types, value);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Bottom, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ColumnWidth, strategy, widthValue);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ExpandStrategy, strategy, minRow, columnCount);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(GridItemPos, strategy, columnId, rowId);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Height, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Left, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MaxHeight, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MaxWidth, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MinHeight, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MinWidth, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Position, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Right, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RowHeight, strategy, fixedValue);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Top, value);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Width, value);

// NOLINTBEGIN
void from_json(const nlohmann::json& json, FlexboxItem& obj);
void from_json(const nlohmann::json& json, GridItem& obj);
void from_json(const nlohmann::json& json, Padding& obj);
void to_json(nlohmann::json& json, const FlexboxItem& obj);
void to_json(nlohmann::json& json, const GridItem& obj);
void to_json(nlohmann::json& json, const Padding& obj);
// NOLINTEND

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
  FlexboxLayout,
  direction,
  justifyContent,
  alignItems,
  alignContent,
  wrap,
  rowGap,
  columnGap,
  padding,
  zOrder);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
  GridLayout,
  expandStrategy,
  columnWidth,
  rowHeight,
  baseHeight,
  columnGap,
  rowGap,
  gridAutoFlow,
  padding,
  cellAlign);

// NOLINTBEGIN
void from_json(const nlohmann::json& json, Rule& obj);
void to_json(nlohmann::json& json, const Rule& obj);
// NOLINTEND

} // namespace Rule
} // namespace Internal
} // namespace Layout
} // namespace VGG
