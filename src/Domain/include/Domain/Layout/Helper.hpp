#pragma once

#include "Rect.hpp"

#include "nlohmann/json.hpp"

namespace VGG
{
namespace Layout
{

void to_json(nlohmann::json& j, const Rect& rect);
void from_json(const nlohmann::json& j, Rect& rect);

bool is_layout_node(const nlohmann::json& json);

} // namespace Layout
} // namespace VGG