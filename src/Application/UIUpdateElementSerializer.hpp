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

#pragma once

#include "Application/UIUpdateElement.hpp"
#include <nlohmann/json.hpp>

namespace VGG::app
{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementFillBlendMode, type, id, index, mode);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
  UpdateElementFillColor,
  type,
  id,
  index,
  a,
  r,
  g,
  b);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementFillEnabled, type, id, index, enabled);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementFillOpacity, type, id, index, opacity);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementFillRotation, type, id, index, degree);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementMatrix, type, id, a, b, c, d, tx, ty);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementOpacity, type, id, opacity);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementVisible, type, id, visible);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(UpdateElementSize, type, id, width, height);

} // namespace VGG::app

namespace nlohmann
{
template<>
struct adl_serializer<VGG::app::UpdateElementItem>
{
  static void from_json(const json& j, VGG::app::UpdateElementItem& x);
};

inline void adl_serializer<VGG::app::UpdateElementItem>::from_json(
  const json&                  j,
  VGG::app::UpdateElementItem& x)
{
  using namespace VGG::app;

  if (!j.is_object())
    throw std::runtime_error("Could not deserialise!");

  const auto& t = j["type"];
  if (t == "fillBlendMode")
    x = j.get<UpdateElementFillBlendMode>();
  else if (t == "fillColor")
    x = j.get<UpdateElementFillColor>();
  else if (t == "fillEnabled")
    x = j.get<UpdateElementFillEnabled>();
  else if (t == "fillOpacity")
    x = j.get<UpdateElementFillOpacity>();
  else if (t == "fillRotation")
    x = j.get<UpdateElementFillRotation>();
  else if (t == "matrix")
    x = j.get<UpdateElementMatrix>();
  else if (t == "opacity")
    x = j.get<UpdateElementOpacity>();
  else if (t == "visible")
    x = j.get<UpdateElementVisible>();
  else if (t == "size")
    x = j.get<UpdateElementSize>();
}

} // namespace nlohmann