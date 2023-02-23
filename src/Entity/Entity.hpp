/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
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
#ifndef __ENTITY_HPP__
#define __ENTITY_HPP__

#include <nlohmann/json.hpp>
#include "Entity/EntityDefs.hpp"
#include "Entity/ComponentSet.hpp"

namespace VGG
{

class EntityContainer;

struct Entity
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Entity, components, visible);

  EntityRaw parent{ nullptr };
  EntityContainer* container{ nullptr };
  ComponentSet components;
  bool visible{ true };

  template<typename T>
  inline Entity& renderable(T&& c)
  {
    components.renderable.emplace<T>(std::move(c));
    return *this;
  }

  template<typename T>
  inline Entity& renderable(T* pc)
  {
    ASSERT(pc);
    components.renderable.emplace<T>(*pc);
    return *this;
  }

  inline Entity& interactable(Interaction&& itr)
  {
    components.interactable.emplace(std::move(itr));
    return *this;
  }

  inline Entity& interactable(std::function<Interaction(EntityRaw)>&& ctor)
  {
    components.interactable.emplace(ctor(this));
    return *this;
  }

  template<typename EventType>
  inline bool accept(const EventType& evt)
  {
    if (components.hasInteractable())
    {
      auto& itr = components.getInteractable();
      if (auto* cb = itr.getCallback<EventType>())
      {
        return (*cb)(evt, *this, itr);
      }
    }
    return false;
  }

  void update(const Entity& ent)
  {
    components.update(ent.components);
    visible = ent.visible;
  }
};

}; // namespace VGG

#endif // __ENTITY_HPP__
