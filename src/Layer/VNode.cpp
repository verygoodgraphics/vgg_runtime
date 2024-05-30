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
#include "Layer/Core/EventManager.hpp"
#include "Layer/StackTrace.hpp"

namespace VGG::layer
{

void Revalidation::emit(const Bounds& bounds, const glm::mat3& ctm, VNode* node)
{
  const auto mappedBounds = bounds.map(ctm);
  m_boundsArray.push_back(mappedBounds);
  m_bounds.unionWith(mappedBounds);
  DEBUG("Revalidation Region %s", node->dbgInfo.c_str());
}

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

void VNode::update()
{
  if (m_state & UPDATE)
  {
    return;
  }
  EventManager::postEvent({ this, ENodeEvent::UPDATE });
  m_state |= UPDATE;
}

void VNode::invalidate(bool damage)
{
  ScopedState state(*this, TRAVERSALING);
  if (state.wasSet())
  {
    VGG_LOG_DEV(ERROR, VNode, "TRAVERSALING");
    return;
  }
  if (isInvalid() && (!damage || m_state & DAMAGE))
    return;
#ifdef VGG_LAYER_DEBUG
  std::string indent(depth(), '\t');
  VGG_TRACE_DEV(indent + dbgInfo);
  VGG_LOG_DEV(TRACE, VNode, "Invalidate: %s", (indent + dbgInfo).c_str());
#endif
  if (damage && !(m_state & BUBBLE_DAMAGE))
  {
    m_state |= DAMAGE;
    damage = false;
  }
  m_state |= INVALIDATE;
  visitObservers(
    [&](auto& obs)
    {
      if (auto p = obs.lock(); p)
      {
        p->invalidate(damage);
      }
    });
}

const Bounds& VNode::revalidate(Revalidation* inv, const glm::mat3& ctm)
{
  ScopedState state(*this, TRAVERSALING);
  if (state.wasSet())
  {
    VGG_LOG_DEV(ERROR, VNode, "TRAVERSALING");
    return m_bounds;
  }
  if (!isInvalid())
    return m_bounds;
#ifdef VGG_LAYER_DEBUG
  std::string indent(depth(), '\t');
  VGG_TRACE_DEV(indent + dbgInfo);
  VGG_LOG_DEV(TRACE, VNode, "Revalidate: %s", (indent + dbgInfo).c_str());
#endif

  const bool noDamage = !inv || (!(m_state & DAMAGE) && !(m_trait & OVERRIDE_DAMAGE));
  // no dmage means that only the current node don't emit changes if any, it doesn not mean that the
  // children don't emit changes
  if (noDamage)
  {
    m_bounds = onRevalidate(inv, ctm);
  }
  else
  {
    const auto prevBounds = m_bounds;
    auto       override = m_trait & OVERRIDE_DAMAGE ? nullptr : inv;
    m_bounds = onRevalidate(override, ctm);
    inv->emit(prevBounds, ctm, this);
    if (m_bounds != prevBounds)
    {
      inv->emit(m_bounds, ctm, this);
    }
  }

  m_state &= ~(INVALIDATE | DAMAGE);
  return m_bounds;
}

} // namespace VGG::layer
