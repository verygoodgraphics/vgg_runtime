/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
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
#ifndef __COMPONENT_SET_HPP__
#define __COMPONENT_SET_HPP__

#include <nlohmann/json.hpp>
#include <variant>

#include "Components/Null.hpp"
#include "Components/Path.hpp"
#include "Components/Text.hpp"
#include "Components/Relation.hpp"
#include "Components/Interaction.hpp"
#include "Components/SoureCode.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Types.hpp"

namespace VGG
{

struct ComponentSet
{
  std::variant<Null, Frame, FramedPath, FramedText, FramedRelation> renderable;
  std::optional<Interaction> interactable;
  SourceCode code;

  template<typename T>
  inline bool hasRenderable() const
  {
    return std::holds_alternative<T>(renderable);
  }

  template<typename T>
  inline T* getRenderable()
  {
    return std::get_if<T>(&renderable);
  }

  template<typename T>
  inline const T* getRenderable() const
  {
    return std::get_if<T>(&renderable);
  }

  inline bool hasInteractable() const
  {
    return interactable.has_value();
  }

  inline Interaction& getInteractable()
  {
    ASSERT(hasInteractable());
    return interactable.value();
  }

  void update(const ComponentSet& cs)
  {
    renderable = cs.renderable;
    code = cs.code;
  }
};

inline void to_json(nlohmann::json& j, const ComponentSet& cs)
{
  if (cs.hasRenderable<Frame>())
  {
    j = nlohmann::json{
      { "renderableType", "Frame" },
      { "renderable", *(cs.getRenderable<Frame>()) },
    };
  }
  else if (cs.hasRenderable<FramedPath>())
  {
    j = nlohmann::json{
      { "renderableType", "FramedPath" },
      { "renderable", *(cs.getRenderable<FramedPath>()) },
    };
  }
  else if (cs.hasRenderable<FramedText>())
  {
    j = nlohmann::json{
      { "renderableType", "FramedText" },
      { "renderable", *(cs.getRenderable<FramedText>()) },
    };
  }
  else if (cs.hasRenderable<FramedRelation>())
  {
    j = nlohmann::json{
      { "renderableType", "FramedRelation" },
      { "renderable", *(cs.getRenderable<FramedRelation>()) },
    };
  }
  else
  {
    WARN("Unsupported ComponentSet for to_json.");
    return;
  }

  j["code"] = cs.code;
}

inline void from_json(const nlohmann::json& j, ComponentSet& cs)
{
  std::string renderableType = j.value("renderableType", "");
  if (renderableType == "Frame")
  {
    cs.renderable = j.at("renderable").get<Frame>();
  }
  else if (renderableType == "FramedPath")
  {
    cs.renderable = j.at("renderable").get<FramedPath>();
  }
  else if (renderableType == "FramedText")
  {
    cs.renderable = j.at("renderable").get<FramedText>();
  }
  else if (renderableType == "FramedRelation")
  {
    cs.renderable = j.at("renderable").get<FramedRelation>();
  }
  else
  {
    WARN("Invalid data of from_json for ComponentSet: %s", j.dump(4).c_str());
    return;
  }

  cs.code = j.value("code", SourceCode{});
}

}; // namespace VGG

#endif // __COMPONENT_SET_HPP__
