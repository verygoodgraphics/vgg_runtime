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
#ifndef __KEYBOARD_ENTITY_HPP__
#define __KEYBOARD_ENTITY_HPP__

#include "Entity/EntityManager.hpp"

namespace VGG
{

struct KeyboardEntity : Entity
{
  struct KeyboardEvent
  {
    KeyboardEntity* source{ nullptr };
  };

  struct KeyboardPress : KeyboardEvent
  {
    size_t modKey{ 0 };
    int32_t key{ 0 };
  };

  struct KeyboardRelease : KeyboardEvent
  {
    size_t modKey{ 0 };
    int32_t key{ 0 };
  };

  struct KeyboardText : KeyboardEvent
  {
    std::string text;
  };

  template<typename EventType>
  void publish(EventType evt)
  {
    if constexpr (std::is_base_of_v<KeyboardEvent, EventType>)
    {
      evt.source = this;
    }
    EntityManager::accept<EventType>(evt);
  }
};

}; // namespace VGG

#endif // __KEYBOARD_ENTITY_HPP__
