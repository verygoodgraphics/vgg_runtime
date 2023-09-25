#include "Utility/Log.hpp"
#include "VGG/Layer/Zoomer.hpp"
#include "VGG/Layer/Scene.hpp"

#include <core/SkCanvas.h>

void Zoomer::setOwnerScene(Scene* owner)
{
  m_owner = owner;
}

Vec2 Zoomer::doTranslate(float x, float y)
{
  m_offset.x += x;
  m_offset.y += y;
  return translate();
}

float Zoomer::doZoom(float speed, float x, float y)
{
  double dz = speed;
  double z2 = m_zoom * (1 + dz);
  if (z2 > 0.01 && z2 < 100)
  {
    m_offset.x -= (x - m_offset.x) * dz;
    m_offset.y -= (y - m_offset.y) * dz;
    m_zoom += m_zoom * dz;
  }
  return scale();
}

void Zoomer::mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const
{
  float x1 = canvasXY[0];
  float y1 = canvasXY[1];
  x1 = (x1 - m_offset.x) / m_zoom;
  y1 = (y1 - m_offset.y) / m_zoom;
  logicXY[0] = x1;
  logicXY[1] = y1;
}
