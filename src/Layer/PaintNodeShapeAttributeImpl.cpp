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
#include "PaintNodeShapeAttributeImpl.hpp"
#include "Layer/Config.hpp"
#include "Layer/Core/PaintNode.hpp"

namespace VGG::layer
{

Bounds PaintNodeShapeAttributeImpl::onRevalidate(Invalidator* inv, const glm::mat3 & mat)
{
  ASSERT(m_paintNode);
  m_shape = m_paintNode->asVisualShape(0);
  if (!m_shape.isEmpty())
  {
    const auto rect = m_shape.bounds();
    auto       bounds = Bounds{ rect.x(), rect.y(), rect.width(), rect.height() };
    return bounds;
  }
  return Bounds();
}
} // namespace VGG::layer
