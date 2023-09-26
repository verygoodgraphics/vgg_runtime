#pragma once

#include <glm/glm.hpp>
#include <iostream>
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
    return glm::vec2{ p.x + topLeft.x, -(p.y + topLeft.y) };
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
    bottomRight.y = topLeft.y - h;
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
};

struct RoundedBound2 : public Bound2
{
private:
  float m_xRadius{ 0.0f }, m_yRadius{ 0.0f };

public:
  RoundedBound2(float x, float y, float w, float h, float xR, float yR)
    : Bound2(x, y, w, h)
    , m_xRadius(xR)
    , m_yRadius(yR)
  {
  }
  RoundedBound2() = default;
  float xRadius() const
  {
    return m_xRadius;
  }

  float yRadius() const
  {
    return m_yRadius;
  }
};

} // namespace VGG
