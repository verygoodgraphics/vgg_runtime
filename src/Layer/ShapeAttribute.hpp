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

#include "AttributeNode.hpp"
#include "Layer/Core/VShape.hpp"

namespace VGG::layer
{

class PaintNode;
class ShapeAttribute : public Attribute
{
public:
  ShapeAttribute(VRefCnt* cnt, PaintNode* paint = 0)
    : Attribute(cnt)
    , m_paintNode(paint)
  {
  }

  virtual const VShape& getShape() const
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

  Bound onRevalidate();

  VGG_CLASS_MAKE(ShapeAttribute);

private:
  friend class RenderNode;
  VShape     m_shape;
  PaintNode* m_paintNode; // temperature solution
};
} // namespace VGG::layer
