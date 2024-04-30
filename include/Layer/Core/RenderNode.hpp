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

#include "Layer/Core/VNode.hpp"
#include <functional>
class SkPicture;
namespace VGG::layer
{
class Renderer;
class RenderNode : public VNode
{
public:
  struct NodeAtContext
  {
    int   localX;
    int   localY;
    void* userData;
  };

  using NodeVisitor = std::function<void(RenderNode*, const NodeAtContext*)>;
  RenderNode(VRefCnt* cnt, EState flags)
    : VNode(cnt, flags)
  {
  }
  bool hasInval() const
  {
    return isInvalid();
  }
  virtual void   nodeAt(int x, int y, NodeVisitor vistor, void* userData) = 0;
  virtual void   render(Renderer* render) = 0;
  virtual Bounds effectBounds() const = 0;

  virtual SkPicture* picture() const
  {
    return nullptr;
  }
};
} // namespace VGG::layer
