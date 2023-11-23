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

#include "Layer/Core/Transform.hpp"

namespace VGG
{
class Bound
{
  glm::vec2 m_topLeft;
  glm::vec2 m_bottomRight;

public:
  Bound()
    : m_topLeft{ 0, 0 }
    , m_bottomRight{ 0, 0 }
  {
  }
  Bound(float x, float y, float w, float h)
    : m_topLeft{ x, y }
    , m_bottomRight{ x + w, y + h }
  {
  }

  Bound(const glm::vec2& topLeft, float w, float h)
    : m_topLeft(topLeft)
    , m_bottomRight{ topLeft.x + w, topLeft.y + h }
  {
  }

  Bound(const glm::vec2& p1, const glm::vec2& p2)
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

  Bound transform(const layer::Transform& transform) const
  {
    auto  topLeft3 = transform * glm::vec3{ m_topLeft, 1.0 };
    auto  bottomRight3 = transform * glm::vec3{ m_bottomRight, 1.0 };
    Bound newBound;
    newBound.m_topLeft = topLeft3;
    newBound.m_bottomRight = bottomRight3;
    return newBound;
  }

  Bound operator*(const layer::Transform& transform) const
  {
    return this->transform(transform);
  }

  void unionWith(const Bound& bound)
  {
    m_topLeft.x = std::min(m_topLeft.x, bound.m_topLeft.x);
    m_topLeft.y = std::min(m_topLeft.y, bound.m_topLeft.y);
    m_bottomRight.x = std::max(m_bottomRight.x, bound.m_bottomRight.x);
    m_bottomRight.y = std::max(m_bottomRight.y, bound.m_bottomRight.y);
  }

  Bound unionAs(const Bound& bound) const
  {
    auto newBound = *this;
    newBound.unionWith(bound);
    return newBound;
  }

  void intersectWith(const Bound& bound)
  {
    m_topLeft.x = std::max(m_topLeft.x, bound.m_topLeft.x);
    m_topLeft.y = std::max(m_topLeft.y, bound.m_topLeft.y);
    m_bottomRight.x = std::min(m_bottomRight.x, bound.m_bottomRight.x);
    m_bottomRight.y = std::min(m_bottomRight.y, bound.m_bottomRight.y);
  }

  Bound intersectAs(const Bound& bound) const
  {
    auto newBound = *this;
    newBound.intersectWith(bound);
    return newBound;
  }

  bool isIntersectWith(const Bound& bound) const
  {
    auto isect = intersectAs(bound);
    return isect.valid();
  }

  bool isIntersectWithEx(const Bound& bound) const
  {
    auto isect = intersectAs(bound);
    auto size = isect.size();
    return size.x >= 0 && size.y >= 0;
  }

public:
  static Bound makeInfinite()
  {
    Bound b;
    b.m_topLeft.x = std::numeric_limits<float>::lowest();
    b.m_topLeft.y = std::numeric_limits<float>::lowest();
    b.m_bottomRight.x = std::numeric_limits<float>::max();
    b.m_bottomRight.y = std::numeric_limits<float>::max();
    return b;
  }

  static Bound makeBoundLRTB(float l, float r, float t, float b)
  {
    Bound bound;
    bound.m_topLeft = { l, t };
    bound.m_bottomRight = { r, b };
    return bound;
  }
};

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v)
{
  os << "[" << v[0] << ", " << v[1] << "]\n";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const Bound& b)
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
