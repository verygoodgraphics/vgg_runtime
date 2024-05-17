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
#include "Layer/Core/Transform.hpp"

namespace VGG::layer
{

class TransformAttribute : public Attribute
{
public:
  TransformAttribute(VRefCnt* cnt)
    : Attribute(cnt)
  {
  }
  VGG_ATTRIBUTE(Transform, const Transform&, m_transform);
  VGG_CLASS_MAKE(TransformAttribute);

  Bounds onRevalidate() override
  {
    return Bounds{};
  }

private:
  friend class RenderNode;
  Transform m_transform;
};
} // namespace VGG::layer
