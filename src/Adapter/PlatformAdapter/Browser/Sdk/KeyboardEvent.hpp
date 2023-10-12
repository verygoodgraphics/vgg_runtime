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

#include "UIEvent.hpp"
#include "Utility/Log.hpp"

#include <memory>

namespace VGG
{
namespace BrowserAdapter
{

class KeyboardEvent : public UIEvent
{
public:
  // getter
  std::string key() const
  {
    char key = event()->key;

    return { key };
  }

  bool repeat() const
  {
    return event()->repeat;
  }

  bool altkey() const
  {
    return event()->altKey;
  }
  bool ctrlkey() const
  {
    return event()->ctrlKey;
  }
  bool metakey() const
  {
    return event()->metaKey;
  }
  bool shiftkey() const
  {
    return event()->shiftKey;
  }

  // methods

private:
  std::shared_ptr<VGG::KeyboardEvent> event() const
  {
    auto result = getEvent<VGG::KeyboardEvent>();
    ASSERT(result);
    return result;
  }
};

} // namespace BrowserAdapter
} // namespace VGG
