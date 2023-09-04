#include "Rule.hpp"

#include "Log.h"

namespace VGG
{
namespace Layout
{
namespace Internal
{
namespace Rule
{
constexpr auto K_ASPECT_RATIO = "aspect_ratio";
constexpr auto K_BOTTOM = "bottom";
constexpr auto K_CLASS = "class";
constexpr auto K_COLUMN_ALIGN = "column_align";
constexpr auto K_COLUMN_SPAN = "column_span";
constexpr auto K_FLEXBOX_ITEM = "flexbox_item";
constexpr auto K_FLEXBOX_LAYOUT = "flexbox_layout";
constexpr auto K_FLEX_GROW = "flex_grow";
constexpr auto K_GRID_ITEM = "grid_item";
constexpr auto K_GRID_LAYOUT = "grid_layout";
constexpr auto K_HEIGHT = "height";
constexpr auto K_ID = "id";
constexpr auto K_ITEM_IN_LAYOUT = "item_in_layout";
constexpr auto K_ITEM_POS = "item_pos";
constexpr auto K_LAYOUT = "layout";
constexpr auto K_LEFT = "left";
constexpr auto K_MAX_HEIGHT = "max_height";
constexpr auto K_MAX_WIDTH = "max_width";
constexpr auto K_MIN_HEIGHT = "min_height";
constexpr auto K_MIN_WIDTH = "min_width";
constexpr auto K_POSITION = "position";
constexpr auto K_RIGHT = "right";
constexpr auto K_ROW_ALIGN = "row_align";
constexpr auto K_ROW_SPAN = "row_span";
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
  obj.flex_grow = json.at(K_FLEX_GROW);

  getPositionFromJson(json, obj);
}

void to_json(nlohmann::json& json, const GridItem& obj)
{
  ASSERT_MSG(false, "not implemented");
}
void from_json(const nlohmann::json& json, GridItem& obj)
{
  obj.item_pos = json.at(K_ITEM_POS);
  obj.row_span = json.at(K_ROW_SPAN);
  obj.column_span = json.at(K_COLUMN_SPAN);
  obj.position = json.at(K_POSITION);
  obj.row_align = json.at(K_ROW_ALIGN);
  obj.column_align = json.at(K_COLUMN_ALIGN);

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
    auto className = json[K_LAYOUT].at(K_CLASS);
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
      obj.item_in_layout = json.at(K_ITEM_IN_LAYOUT).get<FlexboxItem>();
    }
    else if (className == K_GRID_ITEM)
    {
      obj.item_in_layout = json.at(K_ITEM_IN_LAYOUT).get<GridItem>();
    }
  }

  obj.width = json.at(K_WIDTH);
  if (json.contains(K_MAX_WIDTH))
  {
    obj.max_width = json[K_MAX_WIDTH].get<MaxWidth>();
  }
  if (json.contains(K_MIN_WIDTH))
  {
    obj.min_width = json[K_MIN_WIDTH].get<MinWidth>();
  }

  obj.height = json.at(K_HEIGHT);
  if (json.contains(K_MAX_HEIGHT))
  {
    obj.max_height = json[K_MAX_HEIGHT].get<MaxHeight>();
  }
  if (json.contains(K_MIN_HEIGHT))
  {
    obj.min_height = json[K_MIN_HEIGHT].get<MinHeight>();
  }

  if (json.contains(K_ASPECT_RATIO))
  {
    obj.aspect_ratio = json[K_ASPECT_RATIO].get<double>();
  }
}

} // namespace Rule
} // namespace Internal
} // namespace Layout
} // namespace VGG
