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

#include "DefaultRenderNode.hpp"
#include "Layer/AttributeAccessor.hpp"
#include "Layer/Core/PaintNode.hpp"

namespace VGG::layer
{

class RenderNodeFactory
{
public:
  using Creator = std::function<Ref<RenderObjectAttribute>(VAllocator* alloc, ObjectAttribute*)>;
  static std::pair<Ref<DefaultRenderNode>, std::unique_ptr<Accessor>>
  MakeDefaultRenderNode( // NOLINT
    VAllocator*             alloc,
    PaintNode*              node,
    Ref<TransformAttribute> transform,
    Creator                 creator);

  static std::pair<Ref<DefaultRenderNode>, std::unique_ptr<VectorObjectAttibuteAccessor>>
  MakeVectorRenderNode( // NOLINT
    VAllocator*             alloc,
    PaintNode*              node,
    Ref<TransformAttribute> transform);

  static std::pair<Ref<DefaultRenderNode>, std::unique_ptr<ParagraphAttributeAccessor>>
  MakeParagraphRenderNode( // NOLINT
    VAllocator*             alloc,
    PaintNode*              node,
    Ref<TransformAttribute> transform);
};

}; // namespace VGG::layer
