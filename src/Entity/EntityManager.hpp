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
#ifndef __ENTITY_MANAGER_HPP__
#define __ENTITY_MANAGER_HPP__

#include <functional>

#include "Entity/Entity.hpp"
#include "Utils/EntityContainer.hpp"

namespace VGG
{

class EntityManager
{
public:
  enum InteractionMode
  {
    EDIT = 0,
    CREATE_POLYGON,
    CREATE_OVAL,
    CREATE_STAR,
    CREATE_LINE,
    CREATE_PENCIL,
    CREATE_PEN,
    CREATE_TEXT,
    CREATE_IMAGE,
    RUN,
    NUM_ITRS,
  };
  struct CreationParams
  {
    inline constexpr static double minVertexRadius = 0.0;
    inline static double vertexRadius = 0.0;

    inline constexpr static int minNumEdges = 3;
    inline constexpr static int maxNumEdges = 20;
    inline static int numEdges = 4;

    inline static bool isUpward = false;

    inline constexpr static double minRadiusRatio = 0.01;
    inline constexpr static double maxRadiusRatio = 1.0;
    inline static double radiusRatio = 0.5;

    inline constexpr static int minNumPoints = 3;
    inline constexpr static int maxNumPoints = 50;
    inline static int numPoints = 5;
  };

private:
  bool inited{ false };
  Entity root;
  EntityContainer* entities{ nullptr };
  InteractionMode itrMode{ EDIT };
  Interaction interactions[NUM_ITRS];

  inline Interaction& getCurrentInteraction()
  {
    return interactions[itrMode];
  }

  static EntityManager* getInstance();

public:
  static inline Entity& createEntity()
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    return entities->push(Entity{});
  }

  static inline void map(std::function<void(Entity&)> fn)
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    for (auto& entityPtr : *entities)
    {
      fn(*entityPtr);
    }
  }

  static inline void mapR(std::function<void(Entity&)> fn)
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    for (auto it = entities->rbegin(); it != entities->rend(); it++)
    {
      fn(**it);
    }
  }

  static inline bool scan(std::function<bool(Entity&)> fn)
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    for (auto& entityPtr : *entities)
    {
      if (fn(*entityPtr))
      {
        return true;
      }
    }
    return false;
  }

  static inline bool scanR(std::function<bool(Entity&)> fn)
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    for (auto it = entities->rbegin(); it != entities->rend(); it++)
    {
      if (fn(**it))
      {
        return true;
      }
    }
    return false;
  }

  static inline InteractionMode getInteractionMode()
  {
    return EntityManager::getInstance()->itrMode;
  }

  static inline void setInteractionMode(InteractionMode m)
  {
    EntityManager::getInstance()->itrMode = m;
  }

  static inline void updateOffset()
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    entities->updateOffset(Vec2{ 0, 0 });
  }

  static inline void updateRelations()
  {
    auto entities = EntityManager::getInstance()->entities;
    ASSERT(entities);
    entities->updateRelations();
  }

  static inline EntityContainer* getEntities()
  {
    return EntityManager::getInstance()->entities;
  }

  static inline void setEntities(EntityContainer* entities)
  {
    ASSERT(entities);
    EntityManager::getInstance()->entities = entities;
    updateOffset();
    updateRelations();
  }

  template<typename EventType>
  static inline bool accept(const EventType& evt)
  {
    auto em = EntityManager::getInstance();
    auto& interaction = em->getCurrentInteraction();

    if (auto cb = interaction.getCallback<EventType>())
    {
      return (*cb)(evt, em->root, interaction);
    }
    return false;
  }
};

}; // namespace VGG

#endif // __ENTITY_MANAGER_HPP__
