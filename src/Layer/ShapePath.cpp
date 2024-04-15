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
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VShape.hpp"
#include "ShapePath.hpp"
#include "Math/Math.hpp"

namespace
{

using namespace VGG::layer;
using namespace VGG;
inline ContourPtr fromPolygon(const Polygon& poly)
{
  ContourArray c;
  c.closed = true;
  ASSERT(poly.count >= 3);
  c.reserve(poly.count * 2);
  for (int i = 0; i < poly.count; ++i)
  {
    float angle = -VGG::math::number::Pi / 2 + 2 * i * VGG::math::number::Pi / poly.count;
    float x = (0.5 + cos(angle) * 0.5) * poly.bounds.width();
    float y = (0.5 + sin(angle) * 0.5) * poly.bounds.height();
    c.emplace_back(glm::vec2{ x, y }, poly.radius, std::nullopt, std::nullopt, std::nullopt);
  }
  return std::make_shared<ContourArray>(std::move(c));
}

inline ContourPtr fromStar(const Star& poly)
{
  ContourArray c;
  c.closed = true;
  ASSERT(poly.count >= 3);
  c.reserve(poly.count * 2);
  const auto pointCount = poly.count;
  const auto pi = VGG::math::number::Pi;
  const auto ratio = poly.radius;
  const auto radius = poly.radius;
  const auto w = poly.bounds.width();
  const auto h = poly.bounds.height();
  for (int i = 0; i < pointCount; ++i)
  {
    const float angle = -pi / 2 + 2 * i * pi / pointCount;
    const float angle2 = angle + pi / pointCount;
    const float x1 = (0.5 + (cos(angle) * 0.5)) * w;
    const float y1 = (0.5 + (sin(angle) * 0.5)) * h;
    c.emplace_back(glm::vec2{ x1, y1 }, radius, std::nullopt, std::nullopt, std::nullopt);
    const float x2 = (0.5 + (cos(angle2) * 0.5 * ratio)) * w;
    const float y2 = (0.5 + (sin(angle2) * 0.5 * ratio)) * h;
    c.emplace_back(glm::vec2{ x2, y2 }, radius, std::nullopt, std::nullopt, std::nullopt);
  }
  return std::make_shared<ContourArray>(std::move(c));
}

} // namespace

namespace VGG::layer
{

ShapePath::ShapePath(const Polygon& poly)
{
  m_contour = fromPolygon(poly);
}

ShapePath::ShapePath(const Star& poly)
{
  m_contour = fromStar(poly);
}

} // namespace VGG::layer
