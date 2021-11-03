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
#ifndef __RUN_MODE_INTERACTION_HPP__
#define __RUN_MODE_INTERACTION_HPP__

#include <quickjs/quickjs.h>

#include "Entity/KeyboardEntity.hpp"
#include "Entity/MouseEntity.hpp"
#include "Entity/InputManager.hpp"
#include "Utils/JavaScriptInterpreter.hpp"

namespace VGG
{

struct RunModeInteraction
{
  std::shared_ptr<JavaScriptInterpreter> jsi;

  RunModeInteraction(EntityRaw ent)
    : jsi(std::make_shared<JavaScriptInterpreter>(ent))
  {
  }

  template<typename EventType>
  static bool cb(const EventType& evt, Entity& ent, Interaction& itr)
  {
    if (auto state = itr.getState<RunModeInteraction>())
    {
      auto jsi = state->jsi;
      ASSERT(jsi);

      auto res = jsi->callOnEvent<EventType>(evt);
      if (ent.components.hasRenderable<FramedRelation>())
      {
        auto* fr = ent.components.getRenderable<FramedRelation>();
        ASSERT(fr);
        fr->map(
          [evt](Entity& subEnt)
          {
            if (subEnt.components.hasInteractable())
            {
              return cb(evt, subEnt, subEnt.components.getInteractable());
            }
            return false;
          });
      }
      else // update entity only if not relation
      {
        if (res)
        {
          if (auto entJson = jsi->getEntity())
          {
            ent.update(entJson.value());
          }
        }
      }
    }
    else
    {
      WARN("No run mode interaction found!");
    }
    return false;
  }

  static Interaction create(EntityRaw ent)
  {
    using MouseMove = MouseEntity::MouseMove;
    using MouseClick = MouseEntity::MouseClick;
    using MouseRelease = MouseEntity::MouseRelease;
    using KeyboardPress = KeyboardEntity::KeyboardPress;
    using KeyboardRelease = KeyboardEntity::KeyboardRelease;
    using KeyboardText = KeyboardEntity::KeyboardText;
    using EachFrame = InputManager::EachFrame;

    return Interaction{}
      .initialState(RunModeInteraction(ent))
      .on<EachFrame>(cb<EachFrame>)
      .on<MouseMove>(cb<MouseMove>)
      .on<MouseClick>(cb<MouseClick>)
      .on<MouseRelease>(cb<MouseRelease>)
      .on<KeyboardPress>(cb<KeyboardPress>)
      .on<KeyboardRelease>(cb<KeyboardRelease>)
      .on<KeyboardText>(cb<KeyboardText>);
  }
};

}; // namespace VGG

#endif // __RUN_MODE_INTERACTION_HPP__
