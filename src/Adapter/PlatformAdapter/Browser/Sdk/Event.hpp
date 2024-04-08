/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "Domain/Event.hpp"
#include "Utility/Log.hpp"

namespace VGG
{
namespace BrowserAdapter
{

class Event
{
  using event_type = VGG::Event;

  using map_key_type = int;
  inline static std::unordered_map<map_key_type, std::shared_ptr<event_type>> s_event_map;
  inline static map_key_type s_event_id{ 0 };

protected:
  std::shared_ptr<event_type> m_event_ptr;

  template<class T>
  std::shared_ptr<T> getEvent() const
  {
    ASSERT(m_event_ptr);
    auto result = std::dynamic_pointer_cast<T>(m_event_ptr);
    ASSERT(result);
    return result;
  }

public:
  virtual ~Event() = default;

  // staitc
  static auto store(std::shared_ptr<event_type> event)
  {
    ASSERT(event);
    s_event_map[s_event_id] = event;
    return std::to_string(s_event_id++);
  }

  // getter
  std::string target() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->target();
  }

  std::string type() const
  {
    ASSERT(m_event_ptr);
    return m_event_ptr->type();
  }

  // method
  void preventDefault()
  {
    ASSERT(m_event_ptr);
    m_event_ptr->preventDefault();
  }

  // helper method
  void bindCppEvent(int eventId)
  {
    m_event_ptr = s_event_map[eventId];
    ASSERT(m_event_ptr);

    s_event_map.erase(eventId);
  }
};

} // namespace BrowserAdapter
} // namespace VGG
