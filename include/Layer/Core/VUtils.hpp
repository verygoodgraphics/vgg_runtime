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

#define SWITCH_MAP_ITEM_BEGIN(var)                                                                 \
  switch (var)                                                                                     \
  {

#define SWITCH_MAP_ITEM_DEF(from, to)                                                              \
  case from:                                                                                       \
    return to;

#define SWITCH_MAP_ITEM_DEF_NULL(from) case from:;

#define SWITCH_MAP_ITEM_END(fallback)                                                              \
  default:                                                                                         \
    return fallback;                                                                               \
    }

namespace VGG::layer
{
template<class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace VGG::layer
