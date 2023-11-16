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
private:
  glm::vec2 m_topLeft;
  glm::vec2 m_bottomRight;

public:
  Bound2()
    : m_topLeft{ 0, 0 }
    , m_bottomRight{ 0, 0 }
  {
  }
  Bound2(float x, float y, float w, float h)
    : m_topLeft{ x, y }
    , m_bottomRight{ x + w, y + h }
  {
  }

  Bound2(const glm::vec2& topLeft, float w, float h)
    : m_topLeft(topLeft)
    , m_bottomRight{ topLeft.x + w, topLeft.y + h }
  {
  }

  Bound2(const glm::vec2& p1, const glm::vec2& p2)
  {
    m_topLeft = glm::vec2(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
    m_bottomRight = glm::vec2(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
  }

  void extend(float delta)
  {
    m_topLeft.x -= delta;
    m_topLeft.y -= delta;
    m_bottomRight.y += delta;
    m_bottomRight.x += delta;
  }

  // Bound2(const glm::vec2& p1, const glm::vec2& p2)
  //   : topLeft{ std::min(p1.x, p2.x), std::min(p1.y, p2.y) }
  //   , bottomRight{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) }
  // {
  // }

  // map the given p into the bound coordinate
  glm::vec2 map(const glm::vec2& p) const
  {
    return glm::vec2{ p.x + m_topLeft.x, (p.y + m_topLeft.y) };
  }

  const glm::vec2& topLeft() const
  {
    return m_topLeft;
  }

  glm::vec2& topLeft()
  {
    return m_topLeft;
  }

  const glm::vec2& bottomRight() const
  {
    return m_bottomRight;
  }

  glm::vec2 bottomRight()
  {
    return m_bottomRight;
  }

  float width() const
  {
    return m_bottomRight.x - m_topLeft.x;
  }

  void setWidth(float w)
  {
    m_bottomRight.x = m_topLeft.x + w;
  }

  void setHeight(float h)
  {
    m_bottomRight.y = m_topLeft.y + h;
  }

  float height() const
  {
    return -m_topLeft.y + m_bottomRight.y;
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
    auto topLeft3 = transform * glm::vec3{ m_topLeft, 1.0 };
    auto bottomRight3 = transform * glm::vec3{ m_bottomRight, 1.0 };
    Bound2 newBound;
    newBound.m_topLeft = topLeft3;
    newBound.m_bottomRight = bottomRight3;
    return newBound;
  }

  Bound2 operator*(const glm::mat3& transform) const
  {
    return this->transform(transform);
  }

  void unionWith(const Bound2& bound)
  {
    m_topLeft.x = std::min(m_topLeft.x, bound.m_topLeft.x);
    m_topLeft.y = std::min(m_topLeft.y, bound.m_topLeft.y);
    m_bottomRight.x = std::max(m_bottomRight.x, bound.m_bottomRight.x);
    m_bottomRight.y = std::max(m_bottomRight.y, bound.m_bottomRight.y);
  }

  Bound2 unionAs(const Bound2& bound) const
  {
    auto newBound = *this;
    newBound.unionWith(bound);
    return newBound;
  }

  void intersectWith(const Bound2& bound)
  {
    m_topLeft.x = std::max(m_topLeft.x, bound.m_topLeft.x);
    m_topLeft.y = std::max(m_topLeft.y, bound.m_topLeft.y);
    m_bottomRight.x = std::min(m_bottomRight.x, bound.m_bottomRight.x);
    m_bottomRight.y = std::min(m_bottomRight.y, bound.m_bottomRight.y);
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
    b.m_topLeft.x = std::numeric_limits<float>::lowest();
    b.m_topLeft.y = std::numeric_limits<float>::lowest();
    b.m_bottomRight.x = std::numeric_limits<float>::max();
    b.m_bottomRight.y = std::numeric_limits<float>::max();
    return b;
  }
};

} // namespace VGG
