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
inline ShapeData makeShapeData2(
  const Bounds&   bounds,
  EModelShapeType type,
  const float     radius[4],
  float           cornerSmoothing,
  F&&             fallbackContour)
{
  switch (type)
  {
    case EModelShapeType::CONTOUR:
    {
      auto c = fallbackContour(type);
      c->cornerSmooth = cornerSmoothing;
      return c;
    }
    case EModelShapeType::RECTANGLE:
    {
      if (cornerSmoothing <= 0)
      {
        const auto rect = toSkRect(bounds);
        auto       s = makeShape(radius, rect, cornerSmoothing);
        return std::visit([&](auto&& arg) { return ShapeData(arg); }, s);
      }
      else
      {
        const auto l = bounds.topLeft().x;
        const auto t = bounds.topLeft().y;
        const auto r = bounds.bottomRight().x;

        glm::vec2 corners[4] = { bounds.topLeft(),
                                 { r, t },
                                 { r, t - bounds.height() },
                                 { l, t - bounds.height() } };
        Contour   contour;
        contour.closed = true;
        contour.cornerSmooth = cornerSmoothing;
        for (int i = 0; i < 4; i++)
          contour.emplace_back(corners[i], radius[i], std::nullopt, std::nullopt, std::nullopt);
        return std::make_shared<Contour>(contour);
      }
    }
    case EModelShapeType::ELLIPSE:
    {
      Ellipse oval;
      oval.rect = toSkRect(bounds);
      return oval;
    }
    case EModelShapeType::POLYGON:
    case EModelShapeType::STAR:
    case EModelShapeType::VECTORNETWORK:
      return fallbackContour(type);
    case EModelShapeType::UNKNOWN:
      break;
  }
  return ShapeData();
}
} // namespace VGG::layer
