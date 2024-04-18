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
#include "Layer/Model/Concept.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/PathPatch.h"

#include "Layer/Core/VBounds.hpp"
#include "Layer/Core/VShape.hpp"
#include <string>

#include <nlohmann/json.hpp>
#include <string_view>

using namespace nlohmann;
namespace VGG::layer
{

constexpr auto DEFAULT_FONT_ATTR = R"({
        "length":0,
        "name":"Fira Sans",
        "subFamilyName":"",
        "size":14,
        "fontVariations":[],
        "postScript":"",
        "kerning":true,
        "letterSpacingValue":0,
        "letterSpacingUnit":0,
        "lineSpacingValue":0,
        "lineSpacingUnit":0,
        "fillUseType":0,
        "underline":0,
        "linethrough":false,
        "fontVariantCaps":0,
        "textCase":0,
        "baselineShift":0,
        "baseline":0,
        "horizontalScale":1,
        "verticalScale":1,
        "proportionalSpacing":0,
        "rotate":0,
        "textParagraph":{}
    })";

inline EModelObjectType toModelType(std::string_view klass)
{
  if (klass == "frame")
    return EModelObjectType::FRAME;
  else if (klass == "group")
    return EModelObjectType::GROUP;
  else if (klass == "path")
    return EModelObjectType::PATH;
  else if (klass == "text")
    return EModelObjectType::TEXT;
  else if (klass == "symbolMaster")
    return EModelObjectType::MASTER;
  else if (klass == "image")
    return EModelObjectType::IMAGE;
  else if (klass == "object")
    return EModelObjectType::OBJECT;
  else if (klass == "symbolInstance")
    return EModelObjectType::INSTANCE;
  return EModelObjectType::UNKNOWN;
}

inline EModelShapeType toShapeType(std::string_view klass)
{
  if (klass == "contour")
    return EModelShapeType::CONTOUR;
  else if (klass == "rectangle")
    return EModelShapeType::RECTANGLE;
  else if (klass == "ellipse")
    return EModelShapeType::ELLIPSE;
  else if (klass == "polygon")
    return EModelShapeType::POLYGON;
  else if (klass == "star")
    return EModelShapeType::STAR;
  else if (klass == "vectorNetwork")
    return EModelShapeType::VECTORNETWORK;
  return EModelShapeType::UNKNOWN;
}

template<typename F>
inline ShapeData makeShapeData(
  const Bounds&   bounds,
  EModelShapeType type,
  const float     radius[4],
  float           cornerSmoothing,
  int             pointCount,
  F&&             fallback)
{
  switch (type)
  {
    case EModelShapeType::CONTOUR:
    {
      auto c = fallback(type);
      c->cornerSmooth = cornerSmoothing;
      return c;
    }
    case EModelShapeType::RECTANGLE:
    {
      return Rectangle{ .bounds = bounds,
                        .radius = { radius[0], radius[1], radius[2], radius[3] },
                        .cornerSmoothing = cornerSmoothing };
    }
    case EModelShapeType::ELLIPSE:
    {
      Ellipse oval;
      oval.rect = toSkRect(bounds);
      return oval;
    }
    case EModelShapeType::POLYGON:
    {
      return Polygon{ .bounds = bounds,
                      .radius = radius[0],
                      .count = pointCount,
                      .cornerSmoothing = cornerSmoothing };
    }
    case EModelShapeType::STAR:
    {
      return Star{ .bounds = bounds,
                   .radius = radius[0],
                   .ratio = radius[1],
                   .count = pointCount,
                   .cornerSmoothing = cornerSmoothing };
    }
    case EModelShapeType::VECTORNETWORK:
    {
      auto c = fallback(type);
      c->cornerSmooth = cornerSmoothing;
      return c;
    }
    case EModelShapeType::UNKNOWN:
      DEBUG("unknown shape type");
      break;
  }
  return ShapeData();
}
} // namespace VGG::layer
