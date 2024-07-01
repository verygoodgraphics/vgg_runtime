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

// MARK: - Base struct
struct BaseElementGet
{
  std::string type;
  std::string id;
};

struct BaseElementGetFill : BaseElementGet
{
  std::size_t index = 0;
};

// MARK: - Concrete struct
struct ElementGetFillSize : BaseElementGet
{
};

struct ElementGetFillType : BaseElementGetFill
{
};

struct ElementGetFillEnabled : BaseElementGetFill
{
};

struct ElementGetFillColor : BaseElementGetFill
{
};

struct ElementGetFillOpacity : BaseElementGetFill
{
};

struct ElementGetFillBlendMode : BaseElementGetFill
{
};

struct ElementGetFillPatternType : BaseElementGetFill
{
};

struct ElementGetPatternImageFillRotation : BaseElementGetFill
{
  bool effectOnFill = false;
};

struct ElementGetOpacity : BaseElementGet
{
};

struct ElementGetVisible : BaseElementGet
{
};

struct ElementGetMatrix : BaseElementGet
{
};

struct ElementGetWidth : BaseElementGet
{
};
struct ElementGetHeight : BaseElementGet
{
};

using ElementGetProperty = std::variant<
  std::monostate,
  ElementGetFillBlendMode,
  ElementGetFillColor,
  ElementGetFillEnabled,
  ElementGetFillOpacity,
  ElementGetFillPatternType,
  ElementGetFillSize,
  ElementGetFillType,
  ElementGetMatrix,
  ElementGetOpacity,
  ElementGetPatternImageFillRotation,
  ElementGetVisible,
  ElementGetHeight,
  ElementGetWidth>;

// MARK: - Get Result
struct ElementColor
{
  float a = 0;
  float r = 0;
  float g = 0;
  float b = 0;
};

struct ElementMatrix
{
  double a = 0;
  double b = 0;
  double c = 0;
  double d = 0;
  double tx = 0;
  double ty = 0;
};

using ElementProperty =
  std::variant<bool, int, double, std::size_t, std::string, ElementColor, ElementMatrix>;
} // namespace VGG::app
