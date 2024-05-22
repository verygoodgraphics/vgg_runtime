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

#include "Layer/Core/RenderNode.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/AttributeAccessor.hpp"
#include <complex>

namespace VGG::layer
{

class EffectNode : public RenderNode
{
public:
  EffectNode(VRefCnt* cnt, Ref<RenderNode> child)
    : RenderNode(cnt, EState::INVALIDATE)
    , m_child(std::move(child))
  {
    observe(m_child);
  }

  const Ref<RenderNode>& getChild() const
  {
    return m_child;
  }

  ~EffectNode()
  {
    unobserve(m_child);
  }

private:
  Ref<RenderNode> m_child;
};

} // namespace VGG::layer
