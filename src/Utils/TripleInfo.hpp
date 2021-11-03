/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __TRIPLE_INFO_HPP__
#define __TRIPLE_INFO_HPP__

#include "Entity/MouseEntity.hpp"
#include "Entity/KeyboardEntity.hpp"
#include "Components/Interaction.hpp"

namespace VGG
{

template<typename RenderableType, typename StateType>
struct TripleInfo
{
  using MouseEvent = MouseEntity::MouseEvent;
  using KeyboardEvent = KeyboardEntity::KeyboardEvent;

  static std::tuple<MouseEntity*, RenderableType*, std::shared_ptr<StateType>> withMouse(
    const MouseEvent& evt,
    Entity& ent,
    Interaction& itr)
  {
    auto* mouse = evt.source;
    auto* renderable = ent.components.getRenderable<RenderableType>();
    auto state = itr.getState<StateType>();
    ASSERT(mouse);
    ASSERT(renderable);
    ASSERT(state);
    return std::make_tuple(mouse, renderable, state);
  }

  static std::tuple<KeyboardEntity*, RenderableType*, std::shared_ptr<StateType>> withKeyboard(
    const KeyboardEvent& evt,
    Entity& ent,
    Interaction& itr)
  {
    auto* kb = evt.source;
    auto* renderable = ent.components.getRenderable<RenderableType>();
    auto state = itr.getState<StateType>();
    ASSERT(kb);
    ASSERT(renderable);
    ASSERT(state);
    return std::make_tuple(kb, renderable, state);
  }
};

}; // namespace VGG

#endif // __TRIPLE_INFO_HPP__
