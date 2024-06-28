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

#include <string>
#include <variant>

namespace VGG::app
{

struct ElementUpdate
{
  std::string type;
  std::string id;
};

struct ElementUpdateFill : ElementUpdate
{
  std::size_t index = 0;
};

struct ElementUpdateFillEnabled : ElementUpdateFill
{
  bool enabled = false;
};

struct ElementUpdateFillColor : ElementUpdateFill
{
  float a = 0;
  float r = 0;
  float g = 0;
  float b = 0;
};

struct ElementUpdateFillOpacity : ElementUpdateFill
{
  float opacity = 1;
};

struct ElementUpdateFillBlendMode : ElementUpdateFill
{
  int mode = 0;
};

struct ElementUpdatePatternImageFillRotation : ElementUpdateFill
{
  float degree = 0;
  bool  effectOnFill = false;
};

struct ElementUpdateOpacity : ElementUpdate
{
  float opacity = 1;
};

struct ElementUpdateVisible : ElementUpdate
{
  bool visible = false;
};

struct ElementUpdateMatrix : ElementUpdate
{
  float a = 0;
  float b = 0;
  float c = 0;
  float d = 0;
  float tx = 0;
  float ty = 0;
};

struct ElementUpdateSize : ElementUpdate
{
  float width = 0;
  float height = 0;
};

using ElementUpdateProperty = std::variant<
  std::monostate,
  ElementUpdateFillBlendMode,
  ElementUpdateFillColor,
  ElementUpdateFillEnabled,
  ElementUpdateFillOpacity,
  ElementUpdatePatternImageFillRotation,
  ElementUpdateMatrix,
  ElementUpdateOpacity,
  ElementUpdateVisible,
  ElementUpdateSize>;

} // namespace VGG::app