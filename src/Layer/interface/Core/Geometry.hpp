#pragma once
#include "core/SkMatrix.h"
#include "core/SkRect.h"
#include <glm/glm.hpp>
#include <iostream>
#include <ostream>

namespace VGG
{
struct Bound2
{
  enum class CoordOrigin
  {
    TopLeft,
    BottomLeft,
  };

  glm::vec2 topLeft;
  glm::vec2 bottomRight;
  Bound2()
    : topLeft{ 0, 0 }
    , bottomRight{ 0, 0 }
  {
  }
  Bound2(float x, float y, float w, float h)
    : topLeft{ x, y }
    , bottomRight{ x + w, y - h }
  {
  }

  // Bound2(const glm::vec2& p1, const glm::vec2& p2)
  //   : topLeft{ std::min(p1.x, p2.x), std::min(p1.y, p2.y) }
  //   , bottomRight{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) }
  // {
  // }

  // map the given p into the bound coordinate
  glm::vec2 map(const glm::vec2& p) const
  {
    return glm::vec2{ p.x - topLeft.x, p.y + topLeft.y };
  }

  float width() const
  {
    return bottomRight.x - topLeft.x;
  }

  float height() const
  {
    return topLeft.y - bottomRight.y;
  }

  bool valid() const
  {
    return size().x > 0 && size().y > 0;
  }

  glm::vec2 size() const
  {
    return bottomRight - topLeft;
  }
};

// inline Bound2 operator*(const glm::mat3& transform, const Bound2& other)
// {
//   return Bound2{ transform * glm::vec3{ other.topLeft, 1.0 },
//                  transform * glm::vec3{ other.bottomRight, 1.0 } };
// }

// inline Bound2& operator*=(const glm::mat3& transform, Bound2& other)
// {
//   other = transform * other;
//   return other;
// }

inline std::ostream& operator<<(std::ostream& os, const Bound2& b)
{
  os << "[(" << b.topLeft.x << ", " << b.topLeft.y << "), (" << b.bottomRight.x << ","
     << b.bottomRight.y << ")]" << std::endl;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::mat3 m)
{
  os << "[" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << std::endl;
  os << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << std::endl;
  os << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << "]\n";
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const SkMatrix& m)
{
  os << "[" << m[0] << ", " << m[1] << ", " << m[2] << std::endl;
  os << m[3] << ", " << m[4] << ", " << m[5] << std::endl;
  os << m[6] << ", " << m[7] << ", " << m[8] << "]\n";
  return os;
}
inline std::ostream& operator<<(std::ostream& os, const SkRect& m)
{
  os << "center: " << m.centerX() << ", " << m.centerY() << std::endl;
  os << "x, y" << m.x() << ", " << m.y() << std::endl;
  os << "w, h: " << m.width() << ", " << m.height() << std::endl;
  os << m.top() << " " << m.bottom() << " " << m.left() << " " << m.right() << std::endl;
  return os;
}

} // namespace VGG
