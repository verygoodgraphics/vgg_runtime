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
#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>

#include "Entity/EntityManager.hpp"
#include "Entity/MouseEntity.hpp"
#include "Entity/KeyboardEntity.hpp"
#include "Entity/InputManager.hpp"

namespace VGG
{

EntityManager* EntityManager::getInstance()
{
  using MouseMove = MouseEntity::MouseMove;
  using MouseClick = MouseEntity::MouseClick;
  using MouseRelease = MouseEntity::MouseRelease;
  using KeyboardPress = KeyboardEntity::KeyboardPress;
  using KeyboardText = KeyboardEntity::KeyboardText;
  using EachFrame = InputManager::EachFrame;

  static EntityManager em;

  if (!em.inited)
  {
    em.inited = true;
    em.interactions[RUN]
      .on<EachFrame>(
        [](const EachFrame& evt, Entity& root, Interaction& itr) -> bool
        { return EntityManager::scanR([&](Entity& ent) -> bool { return ent.accept(evt); }); })
      .on<MouseMove>(
        [](const MouseMove& evt, Entity& root, Interaction& itr) -> bool
        { return EntityManager::scanR([&](Entity& ent) -> bool { return ent.accept(evt); }); })
      .on<MouseClick>(
        [](const MouseClick& evt, Entity& root, Interaction& itr) -> bool
        { return EntityManager::scanR([&](Entity& ent) -> bool { return ent.accept(evt); }); })
      .on<MouseRelease>(
        [](const MouseRelease& evt, Entity& root, Interaction& itr) -> bool
        { return EntityManager::scanR([&](Entity& ent) -> bool { return ent.accept(evt); }); })
      .on<KeyboardPress>(
        [](const KeyboardPress& evt, Entity& root, Interaction& itr) -> bool
        { return EntityManager::scanR([&](Entity& ent) -> bool { return ent.accept(evt); }); })
      .on<KeyboardText>(
        [](const KeyboardText& evt, Entity& root, Interaction& itr) -> bool
        { return EntityManager::scanR([&](Entity& ent) -> bool { return ent.accept(evt); }); });
  }
  return &em;
}

}; // namespace VGG
