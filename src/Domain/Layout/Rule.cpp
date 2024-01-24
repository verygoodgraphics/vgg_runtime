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
#include "Rule.hpp"

#include "JsonKeys.hpp"

#include "Utility/Log.hpp"
#include "Utility/VggFloat.hpp"

namespace VGG
{
namespace Layout
{
namespace Internal
{
namespace Rule
{
constexpr auto K_ASPECT_RATIO = "aspectRatio";
constexpr auto K_BOTTOM = "bottom";
constexpr auto K_CLASS = "class";
constexpr auto K_COLUMN_ALIGN = "columnAlign";
constexpr auto K_COLUMN_SPAN = "columnSpan";
constexpr auto K_FLEXBOX_ITEM = "flexboxItem";
constexpr auto K_FLEXBOX_LAYOUT = "flexboxLayout";
constexpr auto K_FLEX_BASIS = "flexBasis";
constexpr auto K_GRID_ITEM = "gridItem";
constexpr auto K_GRID_LAYOUT = "gridLayout";
constexpr auto K_HEIGHT = "height";
constexpr auto K_ID = "id";
constexpr auto K_ITEM_IN_LAYOUT = "itemInLayout";
constexpr auto K_ITEM_POS = "itemPos";
constexpr auto K_LAYOUT = "layout";
constexpr auto K_LEFT = "left";
constexpr auto K_MAX_HEIGHT = "maxHeight";
constexpr auto K_MAX_WIDTH = "maxWidth";
constexpr auto K_MIN_HEIGHT = "minHeight";
constexpr auto K_MIN_WIDTH = "minWidth";
constexpr auto K_POSITION = "position";
constexpr auto K_RIGHT = "right";
constexpr auto K_ROW_ALIGN = "rowAlign";
constexpr auto K_ROW_SPAN = "rowSpan";
constexpr auto K_TOP = "top";
constexpr auto K_VALUE = "value";
constexpr auto K_WIDTH = "width";

template<class T>
void getPositionFromJson(const nlohmann::json& json, T& obj)
{
  obj.position.value = static_cast<Position::ETypes>(json.at(K_POSITION).at(K_VALUE).get<int>());
  if (json.contains(K_TOP))
  {
    obj.top = json[K_TOP].get<Top>();
  }
  if (json.contains(K_RIGHT))
  {
    obj.right = json[K_RIGHT].get<Right>();
  }
  if (json.contains(K_BOTTOM))
  {
    obj.bottom = json[K_BOTTOM].get<Bottom>();
  }
  if (json.contains(K_LEFT))
  {
    obj.left = json[K_LEFT].get<Left>();
  }
}

void to_json(nlohmann::json& json, const FlexboxItem& obj)
{
  ASSERT_MSG(false, "not implemented");
}
void from_json(const nlohmann::json& json, FlexboxItem& obj)
{
  obj.flexBasis = json.at(K_FLEX_BASIS);

  getPositionFromJson(json, obj);
}

void to_json(nlohmann::json& json, const GridItem& obj)
{
  ASSERT_MSG(false, "not implemented");
}
void from_json(const nlohmann::json& json, GridItem& obj)
{
  obj.itemPos = json.at(K_ITEM_POS);
  obj.rowSpan = json.at(K_ROW_SPAN);
  obj.columnSpan = json.at(K_COLUMN_SPAN);
  obj.position = json.at(K_POSITION);
  obj.rowAlign = json.at(K_ROW_ALIGN);
  obj.columnAlign = json.at(K_COLUMN_ALIGN);

  getPositionFromJson(json, obj);
}

void to_json(nlohmann::json& json, const Padding& obj)
{
  ASSERT_MSG(false, "not implemented");
}
void from_json(const nlohmann::json& json, Padding& obj)
{
  obj.top = json.at(0);
  obj.right = json.at(1);
  obj.bottom = json.at(2);
  obj.left = json.at(3);
}

void to_json(nlohmann::json& json, const Rule& obj)
{
  ASSERT_MSG(false, "not implemented");
}
void from_json(const nlohmann::json& json, Rule& obj)
{
  obj.id = json.at(K_ID);

  if (json.contains(K_LAYOUT))
  {
    auto className = json[K_LAYOUT].value(K_CLASS, K_EMPTY_STRING);
    if (className == K_FLEXBOX_LAYOUT)
    {
      obj.layout = json.at(K_LAYOUT).get<FlexboxLayout>();
    }
    else if (className == K_GRID_LAYOUT)
    {
      obj.layout = json.at(K_LAYOUT).get<GridLayout>();
    }
  }

  if (json.contains(K_ITEM_IN_LAYOUT))
  {
    auto className = json[K_ITEM_IN_LAYOUT].at(K_CLASS);
    if (className == K_FLEXBOX_ITEM)
    {
      obj.itemInLayout = json.at(K_ITEM_IN_LAYOUT).get<FlexboxItem>();
    }
    else if (className == K_GRID_ITEM)
    {
      obj.itemInLayout = json.at(K_ITEM_IN_LAYOUT).get<GridItem>();
    }
  }

  obj.width = json.at(K_WIDTH);
  if (json.contains(K_MAX_WIDTH))
  {
    obj.maxWidth = json[K_MAX_WIDTH].get<MaxWidth>();
  }
  if (json.contains(K_MIN_WIDTH))
  {
    obj.minWidth = json[K_MIN_WIDTH].get<MinWidth>();
  }

  obj.height = json.at(K_HEIGHT);
  if (json.contains(K_MAX_HEIGHT))
  {
    obj.maxHeight = json[K_MAX_HEIGHT].get<MaxHeight>();
  }
  if (json.contains(K_MIN_HEIGHT))
  {
    obj.minHeight = json[K_MIN_HEIGHT].get<MinHeight>();
  }

  if (json.contains(K_ASPECT_RATIO))
  {
    obj.aspectRatio = json[K_ASPECT_RATIO].get<double>();
  }
}

bool Length::is100Percent()
{
  return types == ETypes::PERCENT && doublesNearlyEqual(value, 100);
}

} // namespace Rule
} // namespace Internal
} // namespace Layout
} // namespace VGG
