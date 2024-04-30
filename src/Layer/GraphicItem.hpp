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
#include "Layer/Core/VNode.hpp"
#include "Layer/Core/RenderNode.hpp"

#include "Layer/Core/VBounds.hpp"

template<typename T>
class sk_sp;
class SkImageFilter;

namespace VGG::layer
{
class ShapeAttribute;
class GraphicItem : public RenderNode
{
public:
  GraphicItem(VRefCnt* cnt)
    : RenderNode(cnt, INVALIDATE)
  {
  }

  void nodeAt(int x, int y, NodeVisitor vistor, void* userData) override
  {
  }

  virtual ShapeAttribute* shape() const = 0;

  virtual sk_sp<SkImageFilter> getMaskFilter() const = 0; // TODO::eliminate this later
};

} // namespace VGG::layer
