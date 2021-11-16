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
#ifndef __ENTITY_CONTAINER_HPP__
#define __ENTITY_CONTAINER_HPP__

#include <nlohmann/json.hpp>
#include <memory>
#include <vector>
#include <unordered_set>
#include <initializer_list>

#include "Entity/EntityDefs.hpp"
#include "Utils/Math.hpp"

namespace VGG
{

class EntityContainer
{
  std::vector<EntityPtr> entities;

  EntityPtr fromEntity(const Entity& ent);

  EntityPtr fromEntityPtr(const EntityPtr& entPtr);

public:
  EntityContainer() = default;

  EntityContainer(std::initializer_list<Entity>&& ls);

  EntityContainer& operator=(const std::vector<Entity>& es);

  EntityContainer(const EntityContainer& other);

  void swap(EntityContainer&& other);

  inline auto size() const
  {
    return entities.size();
  }

  inline auto begin() const
  {
    return entities.begin();
  }

  inline auto begin()
  {
    return entities.begin();
  }

  inline auto end() const
  {
    return entities.end();
  }

  inline auto end()
  {
    return entities.end();
  }

  inline auto rbegin() const
  {
    return entities.rbegin();
  }

  inline auto rbegin()
  {
    return entities.rbegin();
  }

  inline auto rend() const
  {
    return entities.rend();
  }

  inline auto rend()
  {
    return entities.rend();
  }

  inline EntityPtr pop(EntityRaw ent)
  {
    for (auto it = entities.begin(); it != entities.end(); it++)
    {
      EntityPtr p = *it;
      if (p.get() == ent)
      {
        entities.erase(it);
        return p;
      }
    }
    ASSERT_MSG(false, "No such and entity pointer to pop from container.");
  }

  inline Entity& push(const EntityPtr& entPtr)
  {
    entities.push_back(fromEntityPtr(entPtr));
    return *(entities.back());
  }

  inline Entity& push(const Entity& ent)
  {
    return push(fromEntity(ent));
  }

  inline Entity& pushBefore(EntityRaw p, const EntityPtr& entPtr)
  {
    for (auto it = entities.begin(); it != entities.end(); it++)
    {
      auto pe = *it;
      if (pe.get() == p)
      {
        auto t = entities.insert(it, fromEntityPtr(entPtr));
        return *(*t);
      }
    }
    auto t = entities.insert(entities.begin(), fromEntityPtr(entPtr));
    return *(*t);
  }

  inline Entity& pushBefore(EntityRaw p, const Entity& ent)
  {
    return pushBefore(p, fromEntity(ent));
  }

  inline Entity& pushAfter(EntityRaw p, const EntityPtr& entPtr)
  {
    for (auto it = entities.begin(); it != entities.end(); it++)
    {
      auto pe = *it;
      if (pe.get() == p)
      {
        auto t = entities.insert(it + 1, fromEntityPtr(entPtr));
        return *(*t);
      }
    }
    return push(fromEntityPtr(entPtr));
  }

  inline Entity& pushAfter(EntityRaw p, const Entity& ent)
  {
    return pushAfter(p, fromEntity(ent));
  }

  inline Entity& pushAt(size_t idx, const EntityPtr& entPtr)
  {
    auto t = entities.insert(entities.begin() + idx, fromEntityPtr(entPtr));
    return *(*t);
  }

  inline Entity& pushAt(size_t idx, const Entity& ent)
  {
    auto t = entities.insert(entities.begin() + idx, fromEntity(ent));
    return *(*t);
  }

  void updateOffset(const Vec2& offset);

  void applyOffset(const Vec2& offset);

  void setRunModeInteractions(bool init = true);

  void updateRelations(EntityRaw parent = nullptr);

  void collectFillImageNames(std::vector<std::string>& names);

  void collectTypefaceNames(std::unordered_set<std::string>& names);

  bool deleteEntity(EntityRaw ent)
  {
    for (auto it = entities.begin(); it != entities.end(); it++)
    {
      auto p = (*it).get();
      if (p == ent)
      {
        entities.erase(it);
        return true;
      }
    }
    return false;
  }

  bool deleteEntityUpward(EntityRaw ent, EntityRaw parentEnt);

  bool moveBackward(EntityRaw ent);

  bool moveForward(EntityRaw ent);

  inline int getIndex(EntityRaw p)
  {
    if (!p)
    {
      return -1;
    }
    for (size_t i = 0; i < entities.size(); i++)
    {
      if (entities[i].get() == p)
      {
        return i;
      }
    }
    return -1;
  }

  inline EntityPtr get(EntityRaw ent)
  {
    for (auto it = entities.begin(); it != entities.end(); it++)
    {
      EntityPtr p = *it;
      if (p.get() == ent)
      {
        return p;
      }
    }
    ASSERT_MSG(false, "No such and entity pointer to get from container.");
  }
};

void to_json(nlohmann::json& j, const EntityContainer& ec);
void from_json(const nlohmann::json& j, EntityContainer& ec);

}; // namespace VGG

#endif // __ENTITY_CONTAINER_HPP__
