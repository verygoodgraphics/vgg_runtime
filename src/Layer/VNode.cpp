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
#include "Layer/Core/VNode.hpp"

namespace VGG::layer
{
bool VNode::isInvalid() const
{
  return m_state & INVALIDATE;
}

void VNode::observe(VNodePtr sender)
{
  if (sender)
  {
    if (auto it = std::find_if(
          sender->m_observers.begin(),
          sender->m_observers.end(),
          [&](const auto& o) { return o.lock() == this; });
        it == sender->m_observers.end())
    {
      sender->m_observers.push_back(this);
    }
  }
}

void VNode::unobserve(VNodePtr sender)
{
  if (sender)
  {
    if (auto it = std::find_if(
          sender->m_observers.begin(),
          sender->m_observers.end(),
          [&](const auto& o) { return o.lock() == this; });
        it != sender->m_observers.end())
    {
      sender->m_observers.erase(it);
    }
  }
}

void VNode::invalidate()
{
  if (m_state & INVALIDATE)
    return;
  m_state |= INVALIDATE;
  VGG_VNODE_INFO("Invalidate: {}", dbgInfo);
  visitObservers(
    [](auto& obs)
    {
      if (auto p = obs.lock(); p)
      {
        p->invalidate();
      }
    });
}

const Bounds& VNode::revalidate()
{
  if (!isInvalid())
    return m_bounds;
  VGG_VNODE_INFO("Revalidate: {}", dbgInfo);
  m_bounds = onRevalidate();
  m_state &= ~INVALIDATE;
  return m_bounds;
}

} // namespace VGG::layer
