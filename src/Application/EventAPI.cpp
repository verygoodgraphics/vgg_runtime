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
#include "Event/Keycode.hpp"
#include "Utility/Log.hpp"
#include "Event/EventAPI.hpp"

static std::unique_ptr<EventAPI> g_vggEventAPIImpl = nullptr;

void EventManager::registerEventAPI(std::unique_ptr<EventAPI> impl)
{
  g_vggEventAPIImpl = std::move(impl);
}

EVGGKeymod EventManager::getModState()
{
  ASSERT_MSG(g_vggEventAPIImpl, "Event API is not initialized");
  return g_vggEventAPIImpl->getModState();
}

uint8_t* EventManager::getKeyboardState(int* nums)
{
  ASSERT_MSG(g_vggEventAPIImpl, "Event API is not initialized");
  return g_vggEventAPIImpl->getKeyboardState(nums);
}
