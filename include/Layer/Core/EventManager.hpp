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

#include "Layer/Core/RenderNode.hpp"

#include <queue>

namespace VGG::layer
{

enum class ENodeEvent
{
  UPDATE
};

struct Event
{
  WeakRef<VNode> node;
  ENodeEvent     event;
};

class EventManager
{
public:
  static void postEvent(Event e)
  {
    sharedInstance().m_eventQueue.push(e);
  }

  static void pollEvents();

  EventManager(const EventManager&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;

private:
  EventManager() = default;
  static EventManager& sharedInstance()
  {
    static EventManager s_sharedInstance;
    return s_sharedInstance;
  }
  std::queue<Event> m_eventQueue;
};
} // namespace VGG::layer
