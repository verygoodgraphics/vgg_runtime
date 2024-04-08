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

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

namespace VGG
{

class EventVisitor;

struct Event
{
  virtual ~Event() = default;

  // Getter
  virtual std::string target() = 0;
  virtual std::string type() = 0;

  // Method
  virtual void accept(EventVisitor* visitor) = 0;
  virtual void preventDefault()
  {
    std::cout << "Event::preventDefault called" << std::endl;
  }
};

using EventPtr = std::shared_ptr<Event>;

} // namespace VGG
