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
#ifndef __INTERACTION_HPP__
#define __INTERACTION_HPP__

#include <functional>
#include <typeindex>
#include <any>
#include <unordered_map>
#include <type_traits>
#include <optional>

#include "Entity/EntityDefs.hpp"

namespace VGG
{

struct Interaction
{
  std::unordered_map<std::type_index, std::any> callbacks;
  std::optional<std::any> state{ std::nullopt };

  template<typename EventType>
  using CallbackType = std::function<bool(const EventType&, Entity&, Interaction&)>;

  template<typename EventType>
  Interaction& on(CallbackType<EventType>&& cb)
  {
    callbacks[std::type_index(typeid(EventType))] = std::move(cb);
    return *this;
  }

  template<typename StateType>
  Interaction& initialState(StateType&& is)
  {
    state.emplace(std::make_shared<StateType>(is));
    return *this;
  }

  template<typename EventType>
  CallbackType<EventType>* getCallback()
  {
    auto key = std::type_index(typeid(EventType));
    if (callbacks.find(key) != callbacks.end())
    {
      try
      {
        return std::any_cast<CallbackType<EventType>>(&callbacks[key]);
      }
      catch (const std::bad_any_cast& e)
      {
        return nullptr;
      }
    }
    return nullptr;
  }

  template<typename StateType>
  std::shared_ptr<StateType> getState()
  {
    if (!state.has_value())
    {
      return nullptr;
    }
    try
    {
      auto& v = state.value();
      return std::any_cast<std::shared_ptr<StateType>>(v);
    }
    catch (const std::bad_any_cast& e)
    {
      return nullptr;
    }
  }

  template<typename BaseStateType, typename DerivedStateType, typename... DerivedStateTypes>
  std::shared_ptr<BaseStateType> getDerivedState()
  {
    static_assert(std::is_base_of_v<BaseStateType, DerivedStateType>);
    if (auto s = getState<DerivedStateType>())
    {
      return static_cast<std::shared_ptr<BaseStateType>>(s);
    }
    if constexpr (sizeof...(DerivedStateTypes) > 0)
    {
      return getDerivedState<BaseStateType, DerivedStateTypes...>();
    }
    return getState<BaseStateType>();
  }
};

}; // namespace VGG

#endif // __INTERACTION_HPP__
