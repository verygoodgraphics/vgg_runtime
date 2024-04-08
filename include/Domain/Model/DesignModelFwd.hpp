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

#include <variant>

namespace VGG::Model
{
struct Container;
struct Contour;
struct DesignModel;
struct Ellipse;
struct Frame;
struct Group;
struct Image;
struct Object;
struct OverrideValue;
struct Path;
struct Polygon;
struct Rectangle;
struct ReferencedStyle;
struct Shape;
struct Star;
struct Subshape;
struct SymbolInstance;
struct SymbolInstance;
struct SymbolMaster;
struct SymbolMaster;
struct Text;
struct VectorNetwork;

using ContainerChildType =
  std::variant<std::monostate, Frame, Group, Image, Path, SymbolInstance, SymbolMaster, Text>;
using SubGeometryType = std::variant<
  std::monostate,
  Contour,
  Ellipse,
  Frame,
  Group,
  Image,
  Path,
  Polygon,
  Rectangle,
  Star,
  SymbolInstance,
  SymbolMaster,
  Text,
  VectorNetwork>;
} // namespace VGG::Model