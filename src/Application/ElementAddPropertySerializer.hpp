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

#include "Application/ElementAddProperty.hpp"
#include <nlohmann/json.hpp>

namespace VGG::app
{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ElementAddFill, type, id, index, value);

} // namespace VGG::app

namespace nlohmann
{
template<>
struct adl_serializer<VGG::app::ElementAddProperty>
{
  static void from_json(const json& j, VGG::app::ElementAddProperty& x);
};

inline void adl_serializer<VGG::app::ElementAddProperty>::from_json(
  const json&                   j,
  VGG::app::ElementAddProperty& x)
{
  if (!j.is_object())
    throw std::runtime_error("Could not deserialise!");

  const auto& t = j["type"];
  if (t == "fill")
    x = j.get<VGG::app::ElementAddFill>();
}

} // namespace nlohmann
