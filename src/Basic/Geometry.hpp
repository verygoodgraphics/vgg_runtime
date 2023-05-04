#pragma once
#include <glm/glm.hpp>
#include <iostream>

namespace VGG
{
struct Bound2
{
  glm::vec2 bottomLeft;
  glm::vec2 topRight;
  Bound2()
    : bottomLeft{ 0, 0 }
    , topRight{ 0, 0 }
  {
  }
  Bound2(float x, float y, float w, float h)
    : bottomLeft{ x, y }
    , topRight{ x + w, y + h }
  {
  }

  Bound2(const glm::vec2& p1, const glm::vec2& p2)
    : bottomLeft{ std::min(p1.x, p2.x), std::min(p1.y, p2.y) }
    , topRight{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) }
  {
  }

  float width() const
  {
    return topRight.x - bottomLeft.x;
  }

  float height() const
  {
    return topRight.y - bottomLeft.y;
  }

  bool valid() const
  {
    return size().x > 0 && size().y > 0;
  }

  glm::vec2 size() const
  {
    return topRight - bottomLeft;
  }
};

inline Bound2 operator*(const glm::mat3& transform, const Bound2& other)
{
  return Bound2{ transform * glm::vec3{ other.bottomLeft, 1.0 },
                 transform * glm::vec3{ other.topRight, 1.0 } };
}

inline Bound2& operator*=(const glm::mat3& transform, Bound2& other)
{
  other = transform * other;
  return other;
}

inline std::ostream& operator<<(std::ostream& os, const Bound2& b)
{
  os << "[(" << b.bottomLeft.x << ", " << b.bottomLeft.y << "), (" << b.topRight.x << ","
     << b.topRight.y << ")]" << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::mat3 m)
{
  os << "[" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << std::endl;
  os << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << std::endl;
  os << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << "]\n";
  return os;
}

} // namespace VGG
