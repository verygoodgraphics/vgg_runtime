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

#include "Layer/Core/VUtils.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VShape.hpp"
#include <glm/glm.hpp>

namespace VGG::layer
{

class CoordinateConvert
{
public:
  static void convertCoordinateSystem(SkRect& rect, const glm::mat3& totalMatrix)
  {
    auto bounds = Bounds{ rect.x(), rect.y(), rect.width(), rect.height() };
    convertCoordinateSystem(bounds, totalMatrix);
    rect =
      SkRect::MakeXYWH(bounds.topLeft().x, bounds.topLeft().y, bounds.width(), bounds.height());
  }

  static void convertCoordinateSystem(SkRRect& rect, const glm::mat3& totalMatrix)
  {
    auto innerRect = rect.rect();
    convertCoordinateSystem(innerRect, totalMatrix);
    const SkVector radii[4] = { rect.radii(SkRRect::kUpperLeft_Corner),
                                rect.radii(SkRRect::kUpperRight_Corner),
                                rect.radii(SkRRect::kLowerRight_Corner),
                                rect.radii(SkRRect::kLowerLeft_Corner) };
    rect.setRectRadii(innerRect, radii);
  }

  static void convertCoordinateSystem(Star& star, const glm::mat3& totalMatrix)
  {
    convertCoordinateSystem(star.bounds, totalMatrix);
  }

  static void convertCoordinateSystem(Polygon& star, const glm::mat3& totalMatrix)
  {
    convertCoordinateSystem(star.bounds, totalMatrix);
  }

  static void convertCoordinateSystem(ShapeData& shape, const glm::mat3& totalMatrix)
  {
#define VT(type)                                                                                   \
  auto p = std::get_if<type>(&shape);                                                              \
  p
    if (VT(ContourPtr))
    {
      auto& v = *p;
      CoordinateConvert::convertCoordinateSystem(*v, totalMatrix);
    }
    else if (VT(Rectangle))
    {
      if (auto r = std::get_if<SkRect>(&p->rect); r)
      {
        convertCoordinateSystem(*r, totalMatrix);
      }
      else if (auto r = std::get_if<SkRRect>(&p->rect); r)
      {
        convertCoordinateSystem(*r, totalMatrix);
      }
    }
    else if (VT(Ellipse))
    {
      convertCoordinateSystem(p->rect, totalMatrix);
    }
    else if (VT(Star))
    {
      convertCoordinateSystem(*p, totalMatrix);
    }
    else if (VT(Polygon))
    {
      convertCoordinateSystem(*p, totalMatrix);
    }
    else if (VT(VectorNetwork))
    {
      DEBUG("VectorNetwork not impl");
    }
#undef VT
  }
  static void convertCoordinateSystem(Bounds& bounds, const glm::mat3& totalMatrix)
  {
    auto width = bounds.width();
    auto height = bounds.height();
    auto topLeft = bounds.topLeft();
    auto bottomRight = glm::vec2{ topLeft.x + width, topLeft.y - height };
    CoordinateConvert::convertCoordinateSystem(topLeft, totalMatrix);
    CoordinateConvert::convertCoordinateSystem(bottomRight, totalMatrix);
    bounds = Bounds{ topLeft, bottomRight };
  }
  static void convertCoordinateSystem(float& x, float& y, const glm::mat3& totalMatrix)
  {
    // evaluated form of 'point = totalMatrix * glm::vec3(point, 1.f);'
    y = -y;
  }
  static void convertCoordinateSystem(glm::vec2& point, const glm::mat3& totalMatrix)
  {
    // evaluated form of 'point = totalMatrix * glm::vec3(point, 1.f);'
    point.y = -point.y;
  }

  static void convertCoordinateSystem(ContourArray& contour, const glm::mat3& totalMatrix)
  {
    for (auto& p : contour)
    {
      convertCoordinateSystem(p.point, totalMatrix);
      if (p.from)
      {
        convertCoordinateSystem(p.from.value(), totalMatrix);
      }
      if (p.to)
      {
        convertCoordinateSystem(p.to.value(), totalMatrix);
      }
    }
  }

  static std::pair<glm::mat3, glm::mat3> convertMatrixCoordinate(const glm::mat3& mat)
  {
    glm::mat3 scale = glm::identity<glm::mat3>();
    scale = glm::scale(scale, { 1, -1 });
    return { scale * mat * scale, scale * glm::inverse(mat) * scale };
  }

  static void convertCoordinateSystem(Pattern& pattern, const glm::mat3& totalMatrix)
  {

    std::visit(
      Overloaded{ [](PatternFill& p) { p.rotation = -p.rotation; },
                  [](PatternFit& p) { p.rotation = -p.rotation; },
                  [](PatternStretch& p)
                  {
                    auto newMatrix = convertMatrixCoordinate(p.transform.matrix()).first;
                    p.transform.setMatrix(newMatrix);
                  },
                  [](PatternTile& p) { p.rotation = -p.rotation; } },
      pattern.instance);
  }

  static void convertCoordinateSystem(Gradient& gradient, const glm::mat3& totalMatrix)
  {
    std::visit(
      Overloaded{
        [&](GradientLinear& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
        },
        [&](GradientRadial& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
          if (auto d = std::get_if<glm::vec2>(&p.ellipse); d)
          {
            convertCoordinateSystem(*d, totalMatrix);
          }
        },
        [&](GradientAngular& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
          if (auto d = std::get_if<glm::vec2>(&p.ellipse); d)
          {
            convertCoordinateSystem(*d, totalMatrix);
          }
        },
        [&](GradientDiamond& p)
        {
          convertCoordinateSystem(p.from, totalMatrix);
          convertCoordinateSystem(p.to, totalMatrix);
          if (auto d = std::get_if<glm::vec2>(&p.ellipse); d)
          {
            convertCoordinateSystem(*d, totalMatrix);
          }
        },
      },
      gradient.instance);
  }

  static void convertCoordinateSystem(TextStyleAttr& textStyle, const glm::mat3& totalMatrix)
  {
    if (textStyle.fills)
    {
      for (auto& f : *textStyle.fills)
      {
        std::visit(
          Overloaded{
            [&](Gradient& g) { convertCoordinateSystem(g, totalMatrix); },
            [&](Pattern& p) { convertCoordinateSystem(p, totalMatrix); },
            [&](Color& c) {},
          },
          f.type);
      }
    }
  }

  static void convertCoordinateSystem(Style& style, const glm::mat3& totalMatrix)
  {
    for (auto& b : style.borders)
    {
      std::visit(
        Overloaded{
          [&](Gradient& g) { convertCoordinateSystem(g, totalMatrix); },
          [&](Pattern& p) { convertCoordinateSystem(p, totalMatrix); },
          [&](Color& c) {},
        },
        b.type);
    }
    for (auto& f : style.fills)
    {
      std::visit(
        Overloaded{
          [&](Gradient& g) { convertCoordinateSystem(g, totalMatrix); },
          [&](Pattern& p) { convertCoordinateSystem(p, totalMatrix); },
          [&](Color& c) {},
        },
        f.type);
    }
    for (auto& s : style.innerShadow)
    {
      CoordinateConvert::convertCoordinateSystem(s.offsetX, s.offsetY, totalMatrix);
    }
    for (auto& s : style.dropShadow)
    {
      CoordinateConvert::convertCoordinateSystem(s.offsetX, s.offsetY, totalMatrix);
    }
    for (auto& b : style.layerEffects)
    {
      std::visit(
        Overloaded{
          [](const GaussianBlur& blur) {},
          [](const MotionBlur& blur) {},
          [&](RadialBlur& blur)
          { CoordinateConvert::convertCoordinateSystem(blur.xCenter, blur.yCenter, totalMatrix); },
        },
        b.type);
    }
  }
};
} // namespace VGG::layer
