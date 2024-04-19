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
#include "Layer/Core/TransformNode.hpp"

namespace VGG::layer
{
class ViewportNode : public ClipEffectNode
{
public:
  ViewportNode(VRefCnt* cnt)
    : ClipEffectNode(cnt, nullptr)
  {
  }

  void render(Renderer* r) override
  {
  }

  Bounds effectBounds() const override
  {
    return m_viewport;
  }

  void setViewport(const Bounds& viewport)
  {
    m_viewport = viewport;
    this->invalidate();
  }

  const Bounds& getViewport() const
  {
    return m_viewport;
  }

  Bounds onRevalidate() override
  {
    return m_viewport;
  }

  VGG_CLASS_MAKE(ViewportNode);

private:
  Bounds m_viewport;
};
} // namespace VGG::layer
