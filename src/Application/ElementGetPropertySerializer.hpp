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

#include "Application/ElementGetProperty.hpp"
#include <nlohmann/json.hpp>

namespace VGG::app
{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillBlendMode, type, id, index);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillColor, type, id, index);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillEnabled, type, id, index);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillOpacity, type, id, index);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillPatternType, type, id, index);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillSize, type, id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetFillType, type, id, index);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetMatrix, type, id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetOpacity, type, id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
  ElementGetPatternImageFillRotation,
  type,
  id,
  index,
  effectOnFill);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetVisible, type, id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetHeight, type, id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementGetWidth, type, id);

} // namespace VGG::app

namespace nlohmann
{
template<>
struct adl_serializer<VGG::app::ElementGetProperty>
{
  static void from_json(const json& j, VGG::app::ElementGetProperty& x);
};

inline void adl_serializer<VGG::app::ElementGetProperty>::from_json(
  const json&                   j,
  VGG::app::ElementGetProperty& x)
{
  using namespace VGG::app;

  if (!j.is_object())
    throw std::runtime_error("Could not deserialise!");

  const auto& t = j["type"];
  if (t == "fillBlendMode")
    x = j.get<ElementGetFillBlendMode>();
  else if (t == "fillColor")
    x = j.get<ElementGetFillColor>();
  else if (t == "fillEnabled")
    x = j.get<ElementGetFillEnabled>();
  else if (t == "fillOpacity")
    x = j.get<ElementGetFillOpacity>();
  else if (t == "fillPatternType")
    x = j.get<ElementGetFillPatternType>();
  else if (t == "fillSize")
    x = j.get<ElementGetFillSize>();
  else if (t == "fillType")
    x = j.get<ElementGetFillType>();
  else if (t == "matrix")
    x = j.get<ElementGetMatrix>();
  else if (t == "opacity")
    x = j.get<ElementGetOpacity>();
  else if (t == "patternImageFillRotation")
    x = j.get<ElementGetPatternImageFillRotation>();
  else if (t == "visible")
    x = j.get<ElementGetVisible>();
  else if (t == "height")
    x = j.get<ElementGetHeight>();
  else if (t == "width")
    x = j.get<ElementGetWidth>();
}

} // namespace nlohmann
