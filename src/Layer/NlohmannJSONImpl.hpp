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
#include <nlohmann/json.hpp>

#define NLOHMANN_JSON_MODEL_IMPL(getter, key, type, dft)                                           \
  type get##getter() const                                                                         \
  {                                                                                                \
    return j.value<type>(key, dft);                                                                \
  }

#define NLOHMANN_JSON_OBJECT_MODEL_CLASS_GETTER j.value("class", "");

namespace VGG::layer
{

} // namespace VGG::layer
