#pragma once
#include "Math/Math.hpp"

using namespace VGG;

class SkCanvas;

namespace VGG
{
class Scene;
}

class Zoomer
{
  Scene* m_owner{ nullptr };
  double m_zoom{ 1.0 };
  Vec2 m_offset{ 0.0, 0.0 };

public:
  void setOwnerScene(Scene* owner);

  // dx and dy is in the canvas(texture) space.
  Vec2 doTranslate(float dx, float dy);
  // cx and cy is in the canvas(texture) space
  float doZoom(float speed, float cx, float cy);
  float scale() const
  {
    return m_zoom;
  }

  Vec2 translate() const
  {
    return m_offset;
  }

  void mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const;
};
