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

#include "Domain/Model/DesignModel.hpp"
#include "Domain/Model/DesignModelFwd.hpp"

namespace VGG::Model
{

Subshape::Subshape(const Subshape& other)
  : booleanOperation{ other.booleanOperation }
  , subshapeClass{ other.subshapeClass }
{
  if (other.subGeometry)
  {
    subGeometry = std::make_shared<SubGeometryType>(*other.subGeometry);
  }
}

Subshape& Subshape::operator=(const Subshape& other)
{
  booleanOperation = other.booleanOperation;
  subshapeClass = other.subshapeClass;
  if (other.subGeometry)
  {
    subGeometry = std::make_shared<SubGeometryType>(*other.subGeometry);
  }
  return *this;
}

Container::~Container() = default;

} // namespace VGG::Model

namespace nlohmann
{
void adl_serializer<VGG::Model::SubGeometryType>::from_json(
  const json&                  j,
  VGG::Model::SubGeometryType& x)
{
  using namespace VGG::Model;

  if (!j.is_object())
  {
    throw std::runtime_error("Could not deserialise!");
  }

  auto classType = j.at("class").get<SubGeometryClass>();
  switch (classType)
  {
    case SubGeometryClass::CONTOUR:
      x = j.get<Contour>();
      break;
    case SubGeometryClass::ELLIPSE:
      x = j.get<Ellipse>();
      break;
    case SubGeometryClass::FRAME:
      x = j.get<Frame>();
      break;
    case SubGeometryClass::GROUP:
      x = j.get<Group>();
      break;
    case SubGeometryClass::IMAGE:
      x = j.get<Image>();
      break;
    case SubGeometryClass::PATH:
      x = j.get<Path>();
      break;
    case SubGeometryClass::POLYGON:
      x = j.get<Polygon>();
      break;
    case SubGeometryClass::RECTANGLE:
      x = j.get<Rectangle>();
      break;
    case SubGeometryClass::STAR:
      x = j.get<Star>();
      break;
    case SubGeometryClass::SYMBOL_INSTANCE:
      x = j.get<SymbolInstance>();
      break;
    case SubGeometryClass::SYMBOL_MASTER:
      x = j.get<SymbolMaster>();
      break;
    case SubGeometryClass::TEXT:
      x = j.get<Text>();
      break;
    case SubGeometryClass::VECTOR_NETWORK:
      x = j.get<VectorNetwork>();
      break;
  }
}

void adl_serializer<VGG::Model::SubGeometryType>::to_json(
  json&                              j,
  const VGG::Model::SubGeometryType& x)
{
  using namespace VGG::Model;

  if (auto p = std::get_if<Contour>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Ellipse>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Frame>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Group>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Image>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Path>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Polygon>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Rectangle>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Star>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolInstance>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<SymbolMaster>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<Text>(&x))
  {
    j = *p;
  }
  else if (auto p = std::get_if<VectorNetwork>(&x))
  {
    j = *p;
  }
  else
  {
    throw std::runtime_error("Could not serialise, invalid node!");
  }
}

} // namespace nlohmann