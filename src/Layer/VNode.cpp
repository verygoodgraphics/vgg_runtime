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

class VNode::ScopedState
{
public:
  ScopedState(VNode& node, EStateT flag)
    : m_node(node)
    , m_state(flag)
    , m_isSet(node.m_state & flag)
  {
    node.m_state |= flag;
  }

  ~ScopedState()
  {
    if (!m_isSet)
    {
      m_node.m_state &= ~m_state;
    }
  }

  bool wasSet() const
  {
    return m_isSet;
  }

private:
  VNode&   m_node;
  uint32_t m_state;
  bool     m_isSet;
};

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
  ScopedState state(*this, TRAVERSALING);
  if (state.wasSet())
    return;
#ifdef VGG_LAYER_DEBUG
  VGG_TRACE_DEV(dbgInfo);
#endif
  if (m_state & INVALIDATE)
    return;
  DEBUG("on Invalidate %s", dbgInfo.c_str());
  m_state |= INVALIDATE;
  visitObservers(
    [](auto& obs)
    {
      if (auto p = obs.lock(); p)
      {
        p->invalidate();
      }
    });
}

const Bounds& VNode::revalidate(Invalidator* inv, const glm::mat3& ctm)
{
  ScopedState state(*this, TRAVERSALING);
  if (state.wasSet())
    return m_bounds;
#ifdef VGG_LAYER_DEBUG
  VGG_TRACE_DEV(dbgInfo);
#endif
  if (!isInvalid())
    return m_bounds;
  DEBUG("on Revalidate %s", dbgInfo.c_str());
  m_bounds = onRevalidate();
  m_state &= ~INVALIDATE;
  return m_bounds;
}

} // namespace VGG::layer
