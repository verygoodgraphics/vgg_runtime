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
#include "Layer/Core/VNode.hpp"

namespace VGG::layer
{
bool VNode::isInvalid() const
{
  return m_state & INVALIDATE;
}

void VNode::observe(VNodePtr sender)
{
#ifdef USE_SHARED_PTR
  if (auto it = std::find_if(
        sender->m_observers.begin(),
        sender->m_observers.end(),
        [&](const auto& o) { return o.lock() == shared_from_this(); });
      it == sender->m_observers.end())
  {
    sender->m_observers.push_back(shared_from_this());
  }
#else
  if (auto it = std::find_if(
        sender->m_observers.begin(),
        sender->m_observers.end(),
        [&](const auto& o) { return o.lock() == this; });
      it == sender->m_observers.end())
  {
    this->ref();
    sender->m_observers.push_back(Ref<VNode>(this));
  }
#endif
}

void VNode::unobserve(VNodePtr sender)
{

#ifdef USE_SHARED_PTR
  if (auto it = std::find_if(
        sender->m_observers.begin(),
        sender->m_observers.end(),
        [&](const auto& o) { return o.lock() == shared_from_this(); });
      it != sender->m_observers.end())
  {
    sender->m_observers.erase(it);
  }
#else
  if (auto it = std::find_if(
        sender->m_observers.begin(),
        sender->m_observers.end(),
        [&](const auto& o) { return o.lock() == this; });
      it != sender->m_observers.end())
  {
    sender->m_observers.erase(it);
  }
#endif
}

void VNode::invalidate()
{
  if (m_state & INVALIDATE)
    return;
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
const Bound& VNode::revalidate()
{
  if (!isInvalid())
    return m_bound;
  m_bound = onRevalidate();
  m_state &= ~INVALIDATE;
  return m_bound;
}

} // namespace VGG::layer
