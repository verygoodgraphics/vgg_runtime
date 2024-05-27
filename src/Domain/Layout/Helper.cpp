/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Helper.hpp"
#include "JsonKeys.hpp"
#include "Rect.hpp"
#include "Math.hpp"
#include <nlohmann/json.hpp>

namespace VGG
{
namespace Layout
{

enum class EPathType
{
  RECTANGLE,
  LINE,
  ARROW,
  ELLIPSE,
  POLYGON,
  STAR,
  VECTOR
};

void to_json(nlohmann::json& j, const Point& point)
{
  j = { point.x, point.y };
}
void from_json(const nlohmann::json& j, Point& point)
{
  point = { j[0], j[1] };
}

void to_json(nlohmann::json& j, const Rect& rect)
{
  j[K_X] = rect.origin.x;
  j[K_Y] = rect.origin.y;
  j[K_WIDTH] = rect.size.width;
  j[K_HEIGHT] = rect.size.height;
}

void from_json(const nlohmann::json& j, Rect& rect)
{
  rect.origin.x = j.value(K_X, 0.); // double
  rect.origin.y = j.value(K_Y, 0.);
  rect.size.width = j.value(K_WIDTH, 0.);
  rect.size.height = j.value(K_HEIGHT, 0.);
}

void to_json(nlohmann::json& j, const Matrix& matrix)
{
  j[0] = matrix.a;
  j[1] = matrix.b;
  j[2] = matrix.c;
  j[3] = matrix.d;
  j[4] = matrix.tx;
  j[5] = matrix.ty;
}

void from_json(const nlohmann::json& json, Matrix& matrix)
{
  if (!json.is_array() && json.size() != 6)
  {
    return;
  }
  matrix.a = json[0];
  matrix.b = json[1];
  matrix.c = json[2];
  matrix.d = json[3];
  matrix.tx = json[4];
  matrix.ty = json[5];
}

void applyOverridesDetail(
  nlohmann::json&           json,
  std::stack<std::string>   reversedPath,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds)
{
  if (reversedPath.empty())
  {
    return;
  }

  auto key = reversedPath.top();

  reversedPath.pop();
  auto isLastKey = reversedPath.empty();
  if (isLastKey)
  {
    applyLeafOverrides(json, key, value, outDirtyNodeIds);
    return;
  }

  if (key == "*")
  {
    for (auto& el : json.items())
    {
      applyOverridesDetail(el.value(), reversedPath, value, outDirtyNodeIds);
    }
  }
  else if (json.is_array())
  {
    auto path = nlohmann::json::json_pointer{ "/" + key };
    if (json.contains(path))
    {
      applyOverridesDetail(json[path], reversedPath, value, outDirtyNodeIds);
    }
  }
  else
  {
    applyOverridesDetail(json[key], reversedPath, value, outDirtyNodeIds);
  }
}

void applyLeafOverrides(
  nlohmann::json&           json,
  const std::string&        key,
  const nlohmann::json&     value,
  std::vector<std::string>& outDirtyNodeIds)
{
  if (value.is_null()) // A null value indicates deletion of the element
  {
    deleteLeafElement(json, key);
    return;
  }

  if (key == "*")
  {
    for (auto& el : json.items())
    {
      el.value() = value;
    }
  }
  else if (json.is_array())
  {
    auto path = nlohmann::json::json_pointer{ "/" + key };
    if (json.contains(path))
    {
      json[path] = value;
    }
  }
  else
  {
    json[key] = value;

    if (key == K_VISIBLE)
    {
      outDirtyNodeIds.push_back(json[K_ID]);
    }
  }
}

void deleteLeafElement(nlohmann::json& json, const std::string& key)
{
  if (key == "*")
  {
    while (true)
    {
      auto it = json.begin();
      if (it == json.end())
      {
        break;
      }

      json.erase(it.key());
    }
  }
  else if (json.is_array())
  {
    auto path = nlohmann::json::json_pointer{ "/" + key };
    if (json.contains(path))
    {
      auto index = std::stoul(key);
      json.erase(index);
    }
  }
  else
  {
    json.erase(key);
  }
}
} // namespace Layout
} // namespace VGG
