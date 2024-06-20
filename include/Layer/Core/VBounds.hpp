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
#include "Layer/Config.hpp"

#include "Layer/Core/Transform.hpp"

#ifdef STD_FORMAT_SUPPORT
#include <format>
#endif

namespace VGG
{
template<typename T>
class BoundsBase
{
  using Vec2T = glm::vec<2, T, glm::defaultp>;
  using Mat3T = glm::mat<3, 3, T, glm::defaultp>;
  Vec2T m_topLeft;
  Vec2T m_bottomRight;

public:
  BoundsBase()
    : m_topLeft{ 0, 0 }
    , m_bottomRight{ 0, 0 }
  {
  }
  BoundsBase(T x, T y, T w, T h)
    : m_topLeft{ x, y }
    , m_bottomRight{ x + w, y + h }
  {
  }

  BoundsBase(const Vec2T& topLeft, T w, T h)
    : m_topLeft(topLeft)
    , m_bottomRight{ topLeft.x + w, topLeft.y + h }
  {
  }

  BoundsBase(const Vec2T& p1, const Vec2T& p2)
  {
    m_topLeft = Vec2T(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
    m_bottomRight = Vec2T(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
  }

  void extend(T delta)
  {
    m_topLeft.x -= delta;
    m_topLeft.y -= delta;
    m_bottomRight.y += delta;
    m_bottomRight.x += delta;
  }

  Vec2T map(const Vec2T& p) const
  {
    return Vec2T{ p.x + m_topLeft.x, (p.y + m_topLeft.y) };
  }

  const Vec2T& topLeft() const
  {
    return m_topLeft;
  }

  T x() const
  {
    return m_topLeft.x;
  }

  T y() const
  {
    return m_topLeft.y;
  }

  T left() const
  {
    return m_topLeft.x;
  }

  T top() const
  {
    return m_topLeft.y;
  }

  T right() const
  {
    return m_bottomRight.x;
  }

  T bottom() const
  {
    return m_bottomRight.y;
  }

  Vec2T bottomRight() const
  {
    return m_bottomRight;
  }

  T width() const
  {
    return m_bottomRight.x - m_topLeft.x;
  }

  void setWidth(T w)
  {
    m_bottomRight.x = m_topLeft.x + w;
  }

  void setHeight(T h)
  {
    m_bottomRight.y = m_topLeft.y + h;
  }

  T height() const
  {
    return m_bottomRight.y - m_topLeft.y;
  }

  bool valid() const
  {
    return size().x > 0 && size().y > 0;
  }

  Vec2T size() const
  {
    return Vec2T{ width(), height() };
  }

  T squaredDistance() const
  {
    return width() * width() + height() * height();
  }

  float distance() const
  {
    return std::sqrt(squaredDistance());
  }

  BoundsBase<float> bounds(const layer::Transform& transform) const
  {
    auto p1 = transform * glm::vec3{ m_topLeft, 1.0 };
    auto p2 = transform * glm::vec3{ glm::vec2{ m_bottomRight.x, m_topLeft.y }, 1.0 };
    auto p3 = transform * glm::vec3{ m_bottomRight, 1.0 };
    auto p4 = transform * glm::vec3{ glm::vec2{ m_topLeft.x, m_bottomRight.y }, 1.0 };
    auto a = std::initializer_list<T>{ p1.x, p2.x, p3.x, p4.x };
    auto b = std::initializer_list<T>{ p1.y, p2.y, p3.y, p4.y };
    const auto [minX, maxX] = std::minmax_element(a.begin(), a.end());
    const auto [minY, maxY] = std::minmax_element(b.begin(), b.end());
    return BoundsBase<float>{ *minX, *minY, *maxX - *minX, *maxY - *minY };
  }

  BoundsBase<float> map(const glm::mat3& mat) const
  {
    auto p1 = mat * glm::vec3{ m_topLeft, 1.0 };
    auto p2 = mat * glm::vec3{ glm::vec2{ m_bottomRight.x, m_topLeft.y }, 1.0 };
    auto p3 = mat * glm::vec3{ m_bottomRight, 1.0 };
    auto p4 = mat * glm::vec3{ glm::vec2{ m_topLeft.x, m_bottomRight.y }, 1.0 };
    auto a = std::initializer_list<float>{ p1.x, p2.x, p3.x, p4.x };
    auto b = std::initializer_list<float>{ p1.y, p2.y, p3.y, p4.y };
    const auto [minX, maxX] = std::minmax_element(a.begin(), a.end());
    const auto [minY, maxY] = std::minmax_element(b.begin(), b.end());
    return BoundsBase<float>{ *minX, *minY, *maxX - *minX, *maxY - *minY };
  }

  bool operator==(const BoundsBase& other) const
  {
    return m_topLeft == other.topLeft() && m_bottomRight == other.bottomRight();
  }

  void unionWith(const BoundsBase& bounds)
  {
    if (!this->valid())
    {
      *this = bounds;
      return;
    }
    m_topLeft.x = std::min(m_topLeft.x, bounds.m_topLeft.x);
    m_topLeft.y = std::min(m_topLeft.y, bounds.m_topLeft.y);
    m_bottomRight.x = std::max(m_bottomRight.x, bounds.m_bottomRight.x);
    m_bottomRight.y = std::max(m_bottomRight.y, bounds.m_bottomRight.y);
  }

  BoundsBase unionAs(const BoundsBase& bounds) const
  {
    auto newBounds = *this;
    newBounds.unionWith(bounds);
    return newBounds;
  }

  void intersectWith(const BoundsBase& bounds)
  {
    m_topLeft.x = std::max(m_topLeft.x, bounds.m_topLeft.x);
    m_topLeft.y = std::max(m_topLeft.y, bounds.m_topLeft.y);
    m_bottomRight.x = std::min(m_bottomRight.x, bounds.m_bottomRight.x);
    m_bottomRight.y = std::min(m_bottomRight.y, bounds.m_bottomRight.y);
  }

  BoundsBase intersectAs(const BoundsBase& bounds) const
  {
    auto newBounds = *this;
    newBounds.intersectWith(bounds);
    return newBounds;
  }

  bool isIntersectWith(const BoundsBase& bounds) const
  {
    auto isect = intersectAs(bounds);
    return isect.valid();
  }

  bool isIntersectWithEx(const BoundsBase& bounds) const
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

  BoundsBase<float> toFloatBounds() const
  {
    return BoundsBase<float>{ (float)m_topLeft.x,
                              (float)m_topLeft.y,
                              (float)width(),
                              (float)height() };
  }

  BoundsBase<int> toIntBounds() const
  {
    return BoundsBase<int>{ static_cast<int>(m_topLeft.x),
                            static_cast<int>(m_topLeft.y),
                            static_cast<int>(width()),
                            static_cast<int>(height()) };
  }

public:
  static BoundsBase makeInfinite()
  {
    BoundsBase b;
    b.m_topLeft.x = std::numeric_limits<T>::lowest();
    b.m_topLeft.y = std::numeric_limits<T>::lowest();
    b.m_bottomRight.x = std::numeric_limits<T>::max();
    b.m_bottomRight.y = std::numeric_limits<T>::max();
    return b;
  }

  static BoundsBase makeBoundsLRTB(T l, T r, T t, T b)
  {
    BoundsBase bounds;
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

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const BoundsBase<T>& b)
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

using Bounds = BoundsBase<float>;
using Boundsi = BoundsBase<int>;

} // namespace VGG
