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
#include "Helper.hpp"

#include "JsonKeys.hpp"
#include "Rect.hpp"

#include <nlohmann/json.hpp>

namespace VGG
{
namespace Layout
{

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
  rect.origin.x = j.value(K_X, 0.f);
  rect.origin.y = j.value(K_Y, 0.f);
  rect.size.width = j.value(K_WIDTH, 0.f);
  rect.size.height = j.value(K_HEIGHT, 0.f);
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

bool isLayoutNode(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    return false;
  }

  auto className = json.value(K_CLASS, "");
  if (className == K_FRAME || className == K_GROUP || className == K_IMAGE || className == K_PATH ||
      className == K_SYMBOL_INSTANCE || className == K_SYMBOL_MASTER || className == K_TEXT)
  {
    return true;
  }

  return false;
}

bool isPointAttrNode(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    return false;
  }

  auto className = json.value(K_CLASS, "");
  if (className == K_POINT_ATTR)
  {
    return true;
  }

  return false;
}

} // namespace Layout
} // namespace VGG
