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
#include "Utility/Log.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Scene.hpp"

#include <core/SkCanvas.h>

void Zoomer::setOwnerScene(Scene* owner)
{
  m_owner = owner;
}

Vec2 Zoomer::doTranslate(float x, float y)
{
  m_offset.x += x;
  m_offset.y += y;
  if (m_owner)
  {
    m_owner->onZoomTranslationChanged(m_offset.x, m_offset.y);
  }
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

  if (m_owner)
  {
    m_owner->onZoomScaleChanged(m_zoom);
    m_owner->onZoomTranslationChanged(m_offset.x, m_offset.y);
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
