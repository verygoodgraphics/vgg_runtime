#include "Helper.hpp"

#include "JsonKeys.hpp"
#include "Rect.hpp"

#include "nlohmann/json.hpp"

namespace VGG
{
namespace Layout
{

void to_json(nlohmann::json& j, const Rect& rect)
{
  j[k_x] = rect.origin.x;
  j[k_y] = rect.origin.y;
  j[k_width] = rect.size.width;
  j[k_height] = rect.size.height;
}

void from_json(const nlohmann::json& j, Rect& rect)
{
  rect.origin.x = j.value(k_x, 0.f);
  rect.origin.y = j.value(k_y, 0.f);
  rect.size.width = j.value(k_width, 0.f);
  rect.size.height = j.value(k_height, 0.f);
}

bool is_layout_node(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    return false;
  }

  auto class_name = json.value(k_class, "");
  if (class_name == k_frame || class_name == k_group || class_name == k_image ||
      class_name == k_path || class_name == k_symbol_instance || class_name == k_symbol_master ||
      class_name == k_text)
  {
    return true;
  }

  return false;
}

} // namespace Layout
} // namespace VGG