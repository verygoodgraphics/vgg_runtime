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

#include <glm/glm.hpp>
#include <iostream>
#include <limits>
#include <ostream>

namespace VGG
{
struct Bound2
{
  glm::vec2 topLeft;
  glm::vec2 bottomRight;
  Bound2()
    : topLeft{ 0, 0 }
    , bottomRight{ 0, 0 }
  {
  }
  Bound2(float x, float y, float w, float h)
    : topLeft{ x, y }
    , bottomRight{ x + w, y + h }
  {
  }

  Bound2(const glm::vec2& topLeft, float w, float h)
    : topLeft(topLeft)
    , bottomRight{ topLeft.x + w, topLeft.y + h }
  {
  }

  Bound2(const glm::vec2& p1, const glm::vec2& p2)
  {
    topLeft = glm::vec2(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
    bottomRight = glm::vec2(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
  }

  void extend(float delta)
  {
    topLeft.x -= delta;
    topLeft.y -= delta;
    bottomRight.y += delta;
    bottomRight.x += delta;
  }

  // Bound2(const glm::vec2& p1, const glm::vec2& p2)
  //   : topLeft{ std::min(p1.x, p2.x), std::min(p1.y, p2.y) }
  //   , bottomRight{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) }
  // {
  // }

  // map the given p into the bound coordinate
  glm::vec2 map(const glm::vec2& p) const
  {
    return glm::vec2{ p.x + topLeft.x, (p.y + topLeft.y) };
  }

  float width() const
  {
    return bottomRight.x - topLeft.x;
  }

  void setWidth(float w)
  {
    bottomRight.x = topLeft.x + w;
  }

  void setHeight(float h)
  {
    bottomRight.y = topLeft.y + h;
  }

  float height() const
  {
    return -topLeft.y + bottomRight.y;
  }

  bool valid() const
  {
    return size().x > 0 && size().y > 0;
  }

  glm::vec2 size() const
  {
    return glm::vec2{ width(), height() };
  }

  float squaredDistance() const
  {
    return width() * width() + height() * height();
  }

  float distance() const
  {
    return std::sqrt(squaredDistance());
  }

  Bound2 transform(const glm::mat3& transform) const
  {
    auto   topLeft3 = transform * glm::vec3{ topLeft, 1.0 };
    auto   bottomRight3 = transform * glm::vec3{ bottomRight, 1.0 };
    Bound2 newBound;
    newBound.topLeft = topLeft3;
    newBound.bottomRight = bottomRight3;
    return newBound;
  }

  Bound2 operator*(const glm::mat3& transform) const
  {
    return this->transform(transform);
  }

  void unionWith(const Bound2& bound)
  {
    topLeft.x = std::min(topLeft.x, bound.topLeft.x);
    topLeft.y = std::min(topLeft.y, bound.topLeft.y);
    bottomRight.x = std::max(bottomRight.x, bound.bottomRight.x);
    bottomRight.y = std::max(bottomRight.y, bound.bottomRight.y);
  }

  Bound2 unionAs(const Bound2& bound) const
  {
    auto newBound = *this;
    newBound.unionWith(bound);
    return newBound;
  }

  void intersectWith(const Bound2& bound)
  {
    topLeft.x = std::max(topLeft.x, bound.topLeft.x);
    topLeft.y = std::max(topLeft.y, bound.topLeft.y);
    bottomRight.x = std::min(bottomRight.x, bound.bottomRight.x);
    bottomRight.y = std::min(bottomRight.y, bound.bottomRight.y);
  }

  Bound2 intersectAs(const Bound2& bound) const
  {
    auto newBound = *this;
    newBound.intersectWith(bound);
    return newBound;
  }

  bool isIntersectWith(const Bound2& bound) const
  {
    auto isect = intersectAs(bound);
    return isect.valid();
  }

  bool isIntersectWithEx(const Bound2& bound) const
  {
    auto isect = intersectAs(bound);
    auto size = isect.size();
    return size.x >= 0 && size.y >= 0;
  }

public:
  static Bound2 makeInfinite()
  {
    Bound2 b;
    b.topLeft.x = std::numeric_limits<float>::lowest();
    b.topLeft.y = std::numeric_limits<float>::lowest();
    b.bottomRight.x = std::numeric_limits<float>::max();
    b.bottomRight.y = std::numeric_limits<float>::max();
    return b;
  }
};

// struct RoundedBound2 : public Bound2
// {
// private:
//   float m_xRadius{ 0.0f }, m_yRadius{ 0.0f };
//
// public:
//   RoundedBound2(float x, float y, float w, float h, float xR, float yR)
//     : Bound2(x, y, w, h)
//     , m_xRadius(xR)
//     , m_yRadius(yR)
//   {
//   }
//   RoundedBound2() = default;
//   float xRadius() const
//   {
//     return m_xRadius;
//   }
//
//   float yRadius() const
//   {
//     return m_yRadius;
//   }
// };

} // namespace VGG
