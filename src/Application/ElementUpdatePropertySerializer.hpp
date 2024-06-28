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

#include "Application/ElementUpdateProperty.hpp"
#include <nlohmann/json.hpp>

namespace VGG::app
{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateFillBlendMode, type, id, index, mode);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
  ElementUpdateFillColor,
  type,
  id,
  index,
  a,
  r,
  g,
  b);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateFillEnabled, type, id, index, enabled);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateFillOpacity, type, id, index, opacity);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
  ElementUpdatePatternImageFillRotation,
  type,
  id,
  index,
  degree,
  effectOnFill);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateMatrix, type, id, a, b, c, d, tx, ty);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateOpacity, type, id, opacity);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateVisible, type, id, visible);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementUpdateSize, type, id, width, height);

} // namespace VGG::app

namespace nlohmann
{
template<>
struct adl_serializer<VGG::app::ElementUpdateProperty>
{
  static void from_json(const json& j, VGG::app::ElementUpdateProperty& x);
};

inline void adl_serializer<VGG::app::ElementUpdateProperty>::from_json(
  const json&                      j,
  VGG::app::ElementUpdateProperty& x)
{
  using namespace VGG::app;

  if (!j.is_object())
    throw std::runtime_error("Could not deserialise!");

  const auto& t = j["type"];
  if (t == "fillBlendMode")
    x = j.get<ElementUpdateFillBlendMode>();
  else if (t == "fillColor")
    x = j.get<ElementUpdateFillColor>();
  else if (t == "fillEnabled")
    x = j.get<ElementUpdateFillEnabled>();
  else if (t == "fillOpacity")
    x = j.get<ElementUpdateFillOpacity>();
  else if (t == "patternImageFillRotation")
    x = j.get<ElementUpdatePatternImageFillRotation>();
  else if (t == "matrix")
    x = j.get<ElementUpdateMatrix>();
  else if (t == "opacity")
    x = j.get<ElementUpdateOpacity>();
  else if (t == "visible")
    x = j.get<ElementUpdateVisible>();
  else if (t == "size")
    x = j.get<ElementUpdateSize>();
}

} // namespace nlohmann