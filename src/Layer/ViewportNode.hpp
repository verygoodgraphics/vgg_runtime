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
#include "Layer/Core/TransformNode.hpp"

namespace VGG::layer
{
class ViewportNode : public TransformNode
{
public:
  ViewportNode(VRefCnt* cnt, float dpi)
    : TransformNode(cnt)
    , m_dpi(dpi)
  {
  }

  void setDPI(float dpi)
  {
    if (dpi == m_dpi)
      return;
    m_dpi = dpi;
    this->invalidate();
  }

  float getDPI() const
  {
    return m_dpi;
  }

  void setViewport(const Bounds& viewport)
  {
    if (m_viewport == viewport)
      return;
    m_viewport = viewport;
    this->invalidate();
  }

  const Bounds& getViewport() const
  {
    return m_viewport;
  }

  bool hasInvalidate() const
  {
    return isInvalid();
  }

  glm::mat3 getMatrix() const override
  {
    return glm::mat3{ m_dpi, 0, 0, 0, m_dpi, 0, 0, 0, 1 };
  }

  Bounds onRevalidate() override
  {
    return m_viewport;
  }
  VGG_CLASS_MAKE(ViewportNode);

private:
  Bounds m_viewport;
  float  m_dpi{ 1.f };
};
} // namespace VGG::layer
