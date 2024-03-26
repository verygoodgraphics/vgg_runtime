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

#include "Domain/Layout/Math.hpp"
#include "Domain/Layout/Rect.hpp"

#include <nlohmann/json.hpp>

#include <stack>

namespace VGG
{
namespace Layout
{

// NOLINTBEGIN
void to_json(nlohmann::json& j, const Point& point);
void from_json(const nlohmann::json& j, Point& point);

void to_json(nlohmann::json& j, const Rect& rect);
void from_json(const nlohmann::json& j, Rect& rect);

void to_json(nlohmann::json& j, const Matrix& matrix);
void from_json(const nlohmann::json& j, Matrix& matrix);
// NOLINTEND

void applyOverridesDetail(
  nlohmann::json&           json,
  std::stack<std::string>   reversedPath,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds);
void applyLeafOverrides(
  nlohmann::json&           json,
  const std::string&        key,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds);
void deleteLeafElement(nlohmann::json& json, const std::string& key);
} // namespace Layout
} // namespace VGG
