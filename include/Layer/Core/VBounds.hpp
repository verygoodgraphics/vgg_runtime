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

#include <algorithm>
#include <glm/glm.hpp>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <ostream>

#include "Layer/Core/Transform.hpp"

namespace VGG
{
class Bounds
{
  glm::vec2 m_topLeft;
  glm::vec2 m_bottomRight;

public:
  Bounds()
    : m_topLeft{ 0, 0 }
    , m_bottomRight{ 0, 0 }
  {
  }
  Bounds(float x, float y, float w, float h)
    : m_topLeft{ x, y }
    , m_bottomRight{ x + w, y + h }
  {
  }

  Bounds(const glm::vec2& topLeft, float w, float h)
    : m_topLeft(topLeft)
    , m_bottomRight{ topLeft.x + w, topLeft.y + h }
  {
  }

  Bounds(const glm::vec2& p1, const glm::vec2& p2)
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
    return m_bottomRight.y - m_topLeft.y;
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

  Bounds bounds(const layer::Transform& transform) const
  {
    auto p1 = transform * glm::vec3{ m_topLeft, 1.0 };
    auto p2 = transform * glm::vec3{ glm::vec2{ m_bottomRight.x, m_topLeft.y }, 1.0 };
    auto p3 = transform * glm::vec3{ m_bottomRight, 1.0 };
    auto p4 = transform * glm::vec3{ glm::vec2{ m_topLeft.x, m_bottomRight.y }, 1.0 };
    auto a = std::initializer_list<float>{ p1.x, p2.x, p3.x, p4.x };
    auto b = std::initializer_list<float>{ p1.y, p2.y, p3.y, p4.y };
    const auto [minX, maxX] = std::minmax_element(a.begin(), a.end());
    const auto [minY, maxY] = std::minmax_element(b.begin(), b.end());
    return Bounds{ *minX, *minY, *maxX - *minX, *maxY - *minY };
  }

  // Bounds operator*(const layer::Transform& transform) const
  // {
  //   return this->transform(transform);
  // }

  bool operator==(const Bounds& other) const
  {
    return m_topLeft == other.topLeft() && m_bottomRight == other.bottomRight();
  }

  void unionWith(const Bounds& bounds)
  {
    m_topLeft.x = std::min(m_topLeft.x, bounds.m_topLeft.x);
    m_topLeft.y = std::min(m_topLeft.y, bounds.m_topLeft.y);
    m_bottomRight.x = std::max(m_bottomRight.x, bounds.m_bottomRight.x);
    m_bottomRight.y = std::max(m_bottomRight.y, bounds.m_bottomRight.y);
  }

  Bounds unionAs(const Bounds& bounds) const
  {
    auto newBounds = *this;
    newBounds.unionWith(bounds);
    return newBounds;
  }

  void intersectWith(const Bounds& bounds)
  {
    m_topLeft.x = std::max(m_topLeft.x, bounds.m_topLeft.x);
    m_topLeft.y = std::max(m_topLeft.y, bounds.m_topLeft.y);
    m_bottomRight.x = std::min(m_bottomRight.x, bounds.m_bottomRight.x);
    m_bottomRight.y = std::min(m_bottomRight.y, bounds.m_bottomRight.y);
  }

  Bounds intersectAs(const Bounds& bounds) const
  {
    auto newBounds = *this;
    newBounds.intersectWith(bounds);
    return newBounds;
  }

  bool isIntersectWith(const Bounds& bounds) const
  {
    auto isect = intersectAs(bounds);
    return isect.valid();
  }

  bool isIntersectWithEx(const Bounds& bounds) const
  {
    auto isect = intersectAs(bounds);
    auto size = isect.size();
    return size.x >= 0 && size.y >= 0;
  }

  bool contains(float x, float y) const
  {
    return x >= m_topLeft.x && x <= m_bottomRight.x && y >= m_topLeft.y && y <= m_bottomRight.y;
  }

  bool contains(const glm::vec2& p) const
  {
    return contains(p.x, p.y);
  }

public:
  static Bounds makeInfinite()
  {
    Bounds b;
    b.m_topLeft.x = std::numeric_limits<float>::lowest();
    b.m_topLeft.y = std::numeric_limits<float>::lowest();
    b.m_bottomRight.x = std::numeric_limits<float>::max();
    b.m_bottomRight.y = std::numeric_limits<float>::max();
    return b;
  }

  static Bounds makeBoundsLRTB(float l, float r, float t, float b)
  {
    Bounds bounds;
    bounds.m_topLeft = { l, t };
    bounds.m_bottomRight = { r, b };
    return bounds;
  }
};

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v)
{
  os << "[" << v[0] << ", " << v[1] << "]\n";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const Bounds& b)
{
  os << "[" << b.topLeft() << ", " << b.bottomRight() << "]" << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::mat3& mat)
{
  const auto& v0 = mat[0];
  const auto& v1 = mat[1];
  const auto& v2 = mat[2];
  os << "[" << v0[0] << ", " << v1[0] << ", " << v2[0] << std::endl;
  os << v0[1] << ", " << v1[1] << ", " << v2[1] << std::endl;
  os << v0[2] << ", " << v1[2] << ", 1]" << std::endl;
  return os;
}

} // namespace VGG
