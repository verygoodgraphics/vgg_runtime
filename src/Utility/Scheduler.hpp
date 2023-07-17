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
#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include <functional>
#include <optional>

namespace VGG
{

struct Scheduler
{
  inline static std::optional<std::function<void()>> onFrameOnce;

  inline static void setOnFrameOnce(std::function<void()>&& cb)
  {
    onFrameOnce.emplace(cb);
  }

  inline static void callOnFrameOnce()
  {
    if (onFrameOnce.has_value())
    {
      onFrameOnce.value()();
      onFrameOnce.reset();
    }
  }
};

}; // namespace VGG

#endif // __SCHEDULER_HPP__
