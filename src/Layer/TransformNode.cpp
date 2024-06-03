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

#include "Renderer.hpp"
#include "Layer/Core/TransformNode.hpp"

namespace VGG::layer
{
void TransformEffectNode::render(Renderer* renderer)
{
  auto canvas = renderer->canvas();
  ASSERT(canvas);
  SkAutoCanvasRestore acr(canvas, true);
  canvas->concat(toSkMatrix(m_transform->getMatrix()));
  m_child->render(renderer);
}

Bounds RasterTransformNode::onRevalidate(Revalidation* inv, const glm::mat3& ctm)
{
  auto   c = getChild();
  auto   t = getTransform();
  Bounds childBounds;
  if (t)
  {
    t->revalidate();
    const auto& matrix = t->getMatrix();
    m_deviceMatrix[2][0] = matrix[2][0];
    m_deviceMatrix[2][1] = matrix[2][1];
    m_rasterMatrix = matrix;
    m_rasterMatrix[2][0] = 0.0f;
    m_rasterMatrix[2][1] = 0.0f;
  }
  if (c)
  {
    if (t)
    {
      const auto& matrix = t->getMatrix();
      childBounds = c->revalidate(inv, glm::mat3{ 1.0f });
      childBounds = childBounds.map(matrix);
    }
    else
    {
      childBounds = c->revalidate(inv, glm::mat3(1.0f));
    }
  }
  return childBounds;
}
} // namespace VGG::layer
