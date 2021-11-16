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
#include <algorithm>

#include "Entity/Entity.hpp"
#include "Presets/Interactions/RunModeInteraction.hpp"
#include "Utils/EntityContainer.hpp"

namespace VGG
{

EntityContainer::EntityContainer(std::initializer_list<Entity>&& ls)
{
  for (auto& e : ls)
  {
    push(e);
  }
}

EntityContainer& EntityContainer::operator=(const std::vector<Entity>& es)
{
  for (auto& e : es)
  {
    push(e);
  }
  return *this;
}

EntityPtr EntityContainer::fromEntity(const Entity& ent)
{
  auto p = std::make_shared<Entity>(ent);
  ASSERT(p);
  p->container = this;
  return p;
}

EntityPtr EntityContainer::fromEntityPtr(const EntityPtr& entPtr)
{
  EntityPtr p = entPtr;
  p->container = this;
  return p;
}

EntityContainer::EntityContainer(const EntityContainer& other)
{
  if (this == &other)
  {
    return;
  }

  this->entities = other.entities;
  for (auto entPtr : this->entities)
  {
    entPtr->container = this;
  }
}

void EntityContainer::swap(EntityContainer&& other)
{
  this->entities.swap(other.entities);
  for (auto entPtr : this->entities)
  {
    entPtr->container = this;
  }
}

void EntityContainer::updateOffset(const Vec2& offset)
{
  for (auto entPtr : entities)
  {
    if (entPtr->components.hasRenderable<FramedPath>())
    {
      auto fp = entPtr->components.getRenderable<FramedPath>();
      ASSERT(fp);
      fp->frame.globalOffset = offset;
    }
    else if (entPtr->components.hasRenderable<FramedText>())
    {
      auto ft = entPtr->components.getRenderable<FramedText>();
      ASSERT(ft);
      ft->frame.globalOffset = offset;
    }
    else if (entPtr->components.hasRenderable<FramedRelation>())
    {
      auto fr = entPtr->components.getRenderable<FramedRelation>();
      ASSERT(fr);
      fr->frame.globalOffset = offset;
      fr->updateOffset();
    }
    else
    {
      WARN("No supported renderable found to update offset with.");
    }
  }
}

void EntityContainer::applyOffset(const Vec2& offset)
{
  for (auto entPtr : entities)
  {
    if (entPtr->components.hasRenderable<FramedPath>())
    {
      auto fp = entPtr->components.getRenderable<FramedPath>();
      ASSERT(fp);
      fp->frame.x += offset.x;
      fp->frame.y += offset.y;
    }
    else if (entPtr->components.hasRenderable<FramedText>())
    {
      auto ft = entPtr->components.getRenderable<FramedText>();
      ASSERT(ft);
      ft->frame.x += offset.x;
      ft->frame.y += offset.y;
    }
    else if (entPtr->components.hasRenderable<FramedRelation>())
    {
      auto fr = entPtr->components.getRenderable<FramedRelation>();
      ASSERT(fr);
      fr->frame.x += offset.x;
      fr->frame.y += offset.y;
      // fr->updateOffset(); // TODO verify
    }
    else
    {
      WARN("No supported renderable found to apply offset to.");
    }
  }
}

void EntityContainer::setRunModeInteractions(bool init)
{
  for (auto entPtr : entities)
  {
    if (entPtr->components.hasRenderable<FramedPath>() ||
        entPtr->components.hasRenderable<FramedText>() ||
        entPtr->components.hasRenderable<FramedRelation>())
    {
      entPtr->interactable([](EntityRaw ent) { return RunModeInteraction::create(ent); });
      if (auto rmi = entPtr->components.getInteractable().getState<RunModeInteraction>())
      {
        auto c = entPtr->components.code.content;
        if (init && !(c.empty()))
        {
          ASSERT(rmi->jsi);
          rmi->jsi->eval(c);
        }
      }
      else
      {
        FAIL("Failed to set run mode interaction for entity: %p", entPtr.get());
      }
    }
    else
    {
      WARN("Unknown renderable to set run mode interaction.");
    }

    // recursivly set run mode interactions
    if (entPtr->components.hasRenderable<FramedRelation>())
    {
      auto fr = entPtr->components.getRenderable<FramedRelation>();
      ASSERT(fr);
      fr->children.setRunModeInteractions(init);
    }
  }
}

void EntityContainer::updateRelations(EntityRaw parent)
{
  for (auto& entPtr : entities)
  {
    entPtr->parent = parent;
    if (entPtr->components.hasRenderable<FramedRelation>())
    {
      auto fr = entPtr->components.getRenderable<FramedRelation>();
      ASSERT(fr);
      fr->entity = entPtr.get();
      fr->children.updateRelations(entPtr.get());
    }
  }
}

void EntityContainer::collectFillImageNames(std::vector<std::string>& names)
{
  for (auto entPtr : entities)
  {
    if (entPtr->components.hasRenderable<FramedPath>())
    {
      auto fp = entPtr->components.getRenderable<FramedPath>();
      ASSERT(fp);
      for (auto& fill : fp->style.fills)
      {
        if (fill.style.imageName.has_value())
        {
          names.push_back(fill.style.imageName.value());
        }
      }
    }
    else if (entPtr->components.hasRenderable<FramedRelation>())
    {
      auto fr = entPtr->components.getRenderable<FramedRelation>();
      ASSERT(fr);
      fr->children.collectFillImageNames(names);
    }
  }
}

void EntityContainer::collectTypefaceNames(std::unordered_set<std::string>& names)
{
  for (auto entPtr : entities)
  {
    if (entPtr->components.hasRenderable<FramedText>())
    {
      auto ft = entPtr->components.getRenderable<FramedText>();
      ASSERT(ft);
      names.insert(ft->style.typeface);
    }
    else if (entPtr->components.hasRenderable<FramedRelation>())
    {
      auto fr = entPtr->components.getRenderable<FramedRelation>();
      ASSERT(fr);
      fr->children.collectTypefaceNames(names);
    }
  }
}

bool EntityContainer::deleteEntityUpward(EntityRaw ent, EntityRaw parentEnt)
{
  if (!deleteEntity(ent))
  {
    return false;
  }
  if (entities.size() < 1)
  {
    if (parentEnt && parentEnt->container)
    {
      auto container = parentEnt->container;
      return container->deleteEntityUpward(parentEnt, parentEnt->parent);
    }
  }
  return true;
}

bool EntityContainer::moveBackward(EntityRaw ent)
{
  for (auto it = entities.begin(); it != entities.end(); it++)
  {
    if (it->get() == ent && it != entities.begin())
    {
      auto preIt = it - 1;
      std::iter_swap(it, preIt);
      return true;
    }
  }
  return false;
}

bool EntityContainer::moveForward(EntityRaw ent)
{
  for (auto it = entities.rbegin(); it != entities.rend(); it++)
  {
    if (it->get() == ent && it != entities.rbegin())
    {
      auto preIt = it - 1;
      std::iter_swap(it, preIt);
      return true;
    }
  }
  return false;
}

void to_json(nlohmann::json& j, const EntityContainer& ec)
{
  j = nlohmann::json::array();
  for (auto it = ec.begin(); it != ec.end(); it++)
  {
    auto entPtr = *it;
    j.push_back(*entPtr);
  }
}

void from_json(const nlohmann::json& j, EntityContainer& ec)
{
  if (!(j.is_array()))
  {
    WARN("Invalid data of from_json for EntityContainer: %s", j.dump(4).c_str());
    return;
  }

  ec = j.get<std::vector<Entity>>();
}

}; // namespace VGG
