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

  void setZoom(double zoom)
  {
    m_zoom = zoom;
  }
  void setOffset(Vec2 offset)
  {
    m_offset = offset;
  }

  void mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const;
};
