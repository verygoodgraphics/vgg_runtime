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
#include "Layer/Model/StructModel.hpp"
#include "Domain/Model/DesignModelFwd.hpp"
#include "Layer/Model/Concept.hpp"
#include <variant>

namespace
{
using namespace VGG::layer;
using namespace VGG::Model;

inline std::pair<EModelObjectType, ModelType> consumeModelType(ContainerChildType& c)
{
  if (std::holds_alternative<Frame>(c))
  {
    return { EModelObjectType::FRAME, std::make_unique<Frame>(std::get<Frame>(c)) };
  }
  else if (std::holds_alternative<Group>(c))
  {
    return { EModelObjectType::GROUP, std::make_unique<Group>(std::get<Group>(c)) };
  }
  else if (std::holds_alternative<Image>(c))
  {
    return { EModelObjectType::IMAGE, std::make_unique<Image>(std::get<Image>(c)) };
  }
  else if (std::holds_alternative<Path>(c))
  {
    return { EModelObjectType::PATH, std::make_unique<Path>(std::get<Path>(c)) };
  }
  else if (std::holds_alternative<SymbolInstance>(c))
  {
    return { EModelObjectType::INSTANCE,
             std::make_unique<SymbolInstance>(std::get<SymbolInstance>(c)) };
  }
  else if (std::holds_alternative<SymbolMaster>(c))
  {
    return { EModelObjectType::MASTER, std::make_unique<SymbolMaster>(std::get<SymbolMaster>(c)) };
  }
  else if (std::holds_alternative<Text>(c))
  {
    return { EModelObjectType::TEXT, std::make_unique<Text>(std::get<Text>(c)) };
  }
  return { EModelObjectType::UNKNOWN, nullptr };
}

inline StructObject dispatchObject(EModelObjectType modelType, ModelType m)
{
  switch (modelType)
  {
    case EModelObjectType::FRAME:
      return StructFrameObject(std::move(m));
    case EModelObjectType::GROUP:
      return StructGroupObject(std::move(m));
    case EModelObjectType::PATH:
      return StructPathObject(std::move(m));
    case EModelObjectType::TEXT:
      return StructTextObject(std::move(m));
    case EModelObjectType::MASTER:
      return StructMasterObject(std::move(m));
    case EModelObjectType::IMAGE:
      return StructImageObject(std::move(m));
    case EModelObjectType::OBJECT:
      return StructObject(std::move(m), EModelObjectType::OBJECT);
    case EModelObjectType::INSTANCE:
      return StructInstanceObject(std::move(m));
    case EModelObjectType::UNKNOWN:
      break;
  }
  return StructObject(std::move(m), EModelObjectType::UNKNOWN);
}

} // namespace

namespace VGG::layer
{
std::vector<StructObject> StructObject::getChildObjects() const
{
  const auto t = getObjectType();
  switch (t)
  {
    case EModelObjectType::FRAME:
    case EModelObjectType::GROUP:
    case EModelObjectType::MASTER:
    {
      std::vector<StructObject> objects;
      auto                      childObjects = static_cast<Container*>(m.get())->childObjects;
      for (auto& o : childObjects)
      {
        auto [modelType, m] = consumeModelType(o);
        objects.emplace_back(dispatchObject(modelType, std::move(m)));
      }
      return objects;
    }
    case EModelObjectType::PATH:
    case EModelObjectType::IMAGE:
    case EModelObjectType::TEXT:
    case EModelObjectType::INSTANCE:
    case EModelObjectType::OBJECT:
    case EModelObjectType::UNKNOWN:
      DEBUG("no objects");
      break;
  }
  return {};
}
//
std::vector<SubShape<StructObject>> StructPathObject::getShapes() const
{
  return {};
}

} // namespace VGG::layer
