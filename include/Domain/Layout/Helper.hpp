#pragma once

#include "Rect.hpp"

#include "nlohmann/json.hpp"

namespace VGG
{
namespace Layout
{
constexpr auto FLIP_Y_FACTOR = -1;

// NOLINTBEGIN
void to_json(nlohmann::json& j, const Point& point);
void from_json(const nlohmann::json& j, Point& point);

void to_json(nlohmann::json& j, const Rect& rect);
void from_json(const nlohmann::json& j, Rect& rect);

void to_json(nlohmann::json& j, const Matrix& matrix);
void from_json(const nlohmann::json& j, Matrix& matrix);
// NOLINTEND

bool isLayoutNode(const nlohmann::json& json);
bool isPointAttrNode(const nlohmann::json& json);

} // namespace Layout
} // namespace VGG