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
#include "Layer/Core/AttributeAccessor.hpp"

namespace VGG::layer
{
class Viewport final : public TransformNode
{
public:
  Viewport(VRefCnt* cnt, float dpi)
    : TransformNode(cnt)
    , m_dpi(dpi)
  {
#ifdef VGG_LAYER_DEBUG
    dbgInfo = "ViewportNode";
#endif
  }

  VGG_ATTRIBUTE(DPI, float, m_dpi);
  VGG_ATTRIBUTE(Viewport, Bounds, m_viewport);

  bool hasInvalidate() const
  {
    return isInvalid();
  }

  glm::mat3 getMatrix() const override
  {
    return glm::mat3{ 1.f / m_dpi, 0, 0, 0, 1.f / m_dpi, 0, 0, 0, 1 };
  }

  glm::mat3 getInversedMatrix() override
  {
    return glm::inverse(getMatrix());
  }

  Bounds onRevalidate(Revalidation* inv, const glm::mat3& mat) override
  {
    return m_viewport;
  }
  VGG_CLASS_MAKE(Viewport);

private:
  Bounds m_viewport;
  float  m_dpi{ 1.f };
};
} // namespace VGG::layer
