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
#pragma once

#include "PlatformAdapter/Native/Sdk/defines.hpp"
#include "Application/UIEvent.hpp"
#include "Event.hpp"

#include <node_api.h>

#include <memory>

namespace VGG
{
namespace NodeAdapter
{

template<class EventAdapterType, class EventType>
class UIEvent : public Event<EventAdapterType, EventType>
{
  using base_type = Event<EventAdapterType, EventType>;

protected:
  static auto properties()
  {
    auto base_properties{ base_type::properties() };

    decltype(base_properties) v;
    v.insert(v.end(), base_properties.begin(), base_properties.end());

    return v;
  }
};

} // namespace NodeAdapter
} // namespace VGG
