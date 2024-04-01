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

#include "Layer/Core/VNode.hpp"
namespace VGG::layer
{
class Renderer;
class RenderNode : public VNode
{
public:
  RenderNode(VRefCnt* cnt, EState flags)
    : VNode(cnt, flags)
  {
  }
  virtual void  render(Renderer* render) = 0;
  virtual void  renderAsMask(Renderer* render) = 0;
  virtual Bound effectBounds() const = 0;
};
} // namespace VGG::layer
