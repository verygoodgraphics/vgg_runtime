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
#ifndef __RELATION_HPP__
#define __RELATION_HPP__

#include <nlohmann/json.hpp>
#include <vector>
#include <functional>

#include "Components/Frame.hpp"
#include "Utils/EntityContainer.hpp"

namespace VGG
{

struct FramedRelation
{
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(FramedRelation, frame, children);

  Frame frame;
  EntityContainer children;
  EntityRaw entity{ nullptr };

  inline void map(std::function<void(Entity&)> fn)
  {
    for (auto& entityPtr : children)
    {
      fn(*entityPtr);
    }
  }

  inline void mapR(std::function<void(Entity&)> fn)
  {
    for (auto it = children.rbegin(); it != children.rend(); it++)
    {
      fn(**it);
    }
  }

  inline bool scan(std::function<bool(Entity&)> fn)
  {
    for (auto& entityPtr : children)
    {
      if (fn(*entityPtr))
      {
        return true;
      }
    }
    return false;
  }

  inline bool scanR(std::function<bool(Entity&)> fn)
  {
    for (auto it = children.rbegin(); it != children.rend(); it++)
    {
      if (fn(**it))
      {
        return true;
      }
    }
    return false;
  }

  inline void updateOffset()
  {
    auto offset = frame.globalPos();
    children.updateOffset(offset);
  }
};

}; // namespace VGG

#endif // __RELATION_HPP__
