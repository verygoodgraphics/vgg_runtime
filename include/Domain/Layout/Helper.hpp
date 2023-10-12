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

#include "Rect.hpp"

#include <nlohmann/json.hpp>

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
