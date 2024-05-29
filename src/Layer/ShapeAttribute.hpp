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

namespace VGG::layer
{

class PaintNode;
class ShapeAttribute : public Attribute
{
public:
  ShapeAttribute(VRefCnt* cnt)
    : Attribute(cnt)
  {
  }

  virtual const VShape& getShape() const = 0;

  VGG_CLASS_MAKE(ShapeAttribute);

private:
  friend class RenderNode;
};

class ShapeAttributeImpl : public ShapeAttribute
{
public:
  ShapeAttributeImpl(VRefCnt* cnt)
    : ShapeAttribute(cnt)
  {
  }
  const VShape& getShape() const override
  {
    return m_shape;
  }

  void setShape(const VShape& shape)
  {
    if (m_shape == shape)
      return;
    m_shape = shape;
    invalidate();
  }

  Bounds onRevalidate(Invalidator* inv, const glm::mat3 & mat) override
  {
    if (m_shape.isEmpty())
    {
      return Bounds{};
    }
    const auto rect = m_shape.bounds();
    return Bounds{ rect.x(), rect.y(), rect.width(), rect.height() };
  }

  VGG_CLASS_MAKE(ShapeAttributeImpl);

protected:
  VShape m_shape;

private:
  friend class RenderNode;
};
} // namespace VGG::layer
