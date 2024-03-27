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
#include "RenderNodeFactory.hpp"

#include "DefaultRenderNode.hpp"
#include "Layer/RenderObjectAttribute.hpp"
#include "VectorObjectAttribute.hpp"
namespace VGG::layer
{

Ref<DefaultRenderNode> RenderNodeFactory::MakeVectorRenderNode( // NOLINT
  VAllocator*             alloc,
  PaintNode*              node,
  Ref<TransformAttribute> transform)
{
  auto shape = ShapeAttribute::Make(alloc, node);
  auto innerShadow = InnerShadowAttribute::Make(alloc, shape);
  auto dropShadow = DropShadowAttribute::Make(alloc, shape);
  auto backgroundBlur = BackgroundBlurAttribute::Make(alloc);
  auto object = ObjectAttribute::Make(alloc, Ref<RenderObjectAttribute>());

  auto vectorObject = VectorObjectAttribute::Make(alloc, shape, object);
  object->setRenderObject(vectorObject);

  auto style = StyleObjectAttribute::Make(alloc, innerShadow, dropShadow, object, backgroundBlur);
  auto layerPostProcess = LayerFXAttribute::Make(alloc, style);
  auto shapeMask = ShapeMaskAttribute::Make(alloc, node, layerPostProcess);
  auto alphaMaskAttribute = AlphaMaskAttribute::Make(alloc, node, layerPostProcess);
  auto result = DefaultRenderNode::Make(
    alloc,
    transform,
    style,
    layerPostProcess,
    alphaMaskAttribute,
    shapeMask,
    shape);
  result->m_accessor = std::unique_ptr<AttributeAccessor>(new AttributeAccessor(
    transform,
    shape,
    alphaMaskAttribute,
    shapeMask,
    dropShadow,
    innerShadow,
    object,
    layerPostProcess,
    backgroundBlur));
  return result;
}

} // namespace VGG::layer