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

#include "AttributeNode.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/ShapeAttribute.hpp"
namespace VGG::layer
{

class PaintNode;
class PaintNodeShapeAttributeImpl : public ShapeAttribute
{
public:
  PaintNodeShapeAttributeImpl(VRefCnt* cnt, PaintNode* paint = 0)
    : ShapeAttribute(cnt)
    , m_paintNode(paint)
  {
    ASSERT(m_paintNode);
  }

  const VShape& getShape() const override
  {
    return m_shape;
  }

  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override;
  VGG_CLASS_MAKE(PaintNodeShapeAttributeImpl);

private:
  VShape m_shape;
  friend class RenderNode;
  PaintNode* m_paintNode; // temperature solution
};
} // namespace VGG::layer
