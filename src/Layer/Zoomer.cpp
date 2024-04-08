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
#include "Utility/Log.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Scene.hpp"

#include <core/SkCanvas.h>

void Zoomer::setOwnerScene(Scene* owner)
{
  m_owner = owner;
  if (m_owner)
  {
    m_owner->onZoomScaleChanged(m_scale);
    m_owner->onZoomTranslationChanged(m_offset.x, m_offset.y);
  }
}

void Zoomer::setOffset(glm::vec2 offset)
{
  m_offset = offset;
  if (m_owner)
  {
    m_owner->onZoomTranslationChanged(m_offset.x, m_offset.y);
  }
}

void Zoomer::setTranslate(float dx, float dy)
{
  setOffset(glm::vec2(dx, dy) + m_offset);
}

// float Zoomer::doZoom(float speed, float x, float y)
// {
//   double dz = speed;
//   double z2 = m_scale.second * (1 + dz);
//   if (z2 > 0.01 && z2 < 100)
//   {
//     m_offset.x -= (x - m_offset.x) * dz;
//     m_offset.y -= (y - m_offset.y) * dz;
//     m_scale.second += m_scale.second * dz;
//   }
//
//   if (m_owner)
//   {
//     m_owner->onZoomScaleChanged(m_scale);
//     m_owner->onZoomTranslationChanged(m_offset.x, m_offset.y);
//   }
//   return scale();
// }

void Zoomer::setScale(EScaleLevel level, glm::vec2 anchor)
{
  if (level >= EScaleLevel::SL_1_4 && level <= EScaleLevel::SL_5_1)
  {
    updateScale({ level, ZOOM_LEVEL[level] }, anchor);
  }
}

bool Zoomer::updateScale(Scale scale, glm::vec2 anchor)
{
  float dz = (scale.second - m_scale.second) / m_scale.second;
  if (scale.second > 0.01 && scale.second < 100)
  {
    m_offset.x -= (anchor.x - m_offset.x) * dz;
    m_offset.y -= (anchor.y - m_offset.y) * dz;
    m_scale = scale;
  }
  else
  {
    return false;
  }

  if (m_owner)
  {
    m_owner->onZoomScaleChanged(m_scale);
    m_owner->onZoomTranslationChanged(m_offset.x, m_offset.y);
  }
  return true;
}

void Zoomer::mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const
{
  float x1 = canvasXY[0];
  float y1 = canvasXY[1];
  x1 = (x1 - m_offset.x) / m_scale.second;
  y1 = (y1 - m_offset.y) / m_scale.second;
  logicXY[0] = x1;
  logicXY[1] = y1;
}
