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

#include "Layer/Model/JSONModel.hpp"

#include <vector>

namespace
{

using namespace VGG::layer;

inline ContourPtr makeContourData2(const json& j)
{
  ContourArray contour;
  contour.closed = j.value("closed", false);
  const auto& points = getOrDefault(j, "points");
  for (const auto& e : points)
  {
    contour.emplace_back(
      getOptional<glm::vec2>(e, "point").value_or(glm::vec2{ 0, 0 }),
      getOptional<float>(e, "radius").value_or(0.0),
      getOptional<glm::vec2>(e, "curveFrom"),
      getOptional<glm::vec2>(e, "curveTo"),
      getOptional<int>(e, "cornerStyle"));
  }
  auto ptr = std::make_shared<ContourArray>(contour);
  if (!ptr->closed && !ptr->empty())
  {
    ptr->back().radius = 0;
    ptr->front().radius = 0;
  }
  return ptr;
}

JSONObject dispatchObject(EModelObjectType modelType, json j)
{
  switch (modelType)
  {
    case EModelObjectType::FRAME:
      return JSONFrameObject(std::move(j));
    case EModelObjectType::GROUP:
      return JSONGroupObject(std::move(j));
    case EModelObjectType::PATH:
      return JSONPathObject(std::move(j));
    case EModelObjectType::TEXT:
      return JSONTextObject(std::move(j));
    case EModelObjectType::MASTER:
      return JSONMasterObject(std::move(j));
    case EModelObjectType::IMAGE:
      return JSONImageObject(std::move(j));
    case EModelObjectType::OBJECT:
      return JSONObject(std::move(j), EModelObjectType::OBJECT);
    case EModelObjectType::INSTANCE:
      return JSONInstanceObject(std::move(j));
    case EModelObjectType::UNKNOWN:
      break;
  }
  return JSONObject(j, EModelObjectType::UNKNOWN);
}
} // namespace

namespace VGG::layer
{

std::vector<JSONObject> JSONObject::getChildObjects() const
{
  std::vector<JSONObject> objects;
  const auto              childObjects = getOrDefault(j, "childObjects");
  for (auto& j : childObjects)
  {
    const auto klass = j.value("class", "");
    auto       modelType = toModelType(klass);
    objects.emplace_back(dispatchObject(modelType, std::move(j)));
  }
  return objects;
}

std::vector<SubShape<JSONObject>> JSONPathObject::getShapes() const
{
  std::vector<SubShape<JSONObject>> res;
  const auto shapes = j.value("shape", json{}).value("subshapes", std::vector<json>());
  const auto bounds = getBounds();
  const auto smooth = getCornerSmoothing();
  for (const auto& subshape : shapes)
  {
    const auto            blop = subshape.value("booleanOperation", EBoolOp::BO_NONE);
    const auto            geo = subshape.value("subGeometry", nlohmann::json{});
    const auto            klass = geo.value("class", "");
    const EModelShapeType shapeType = toShapeType(klass);
    if (shapeType != EModelShapeType::UNKNOWN)
    {
      const auto radius = geo.value("radius", Float4{ 0.f, 0.f, 0.f, 0.f });
      res.emplace_back(
        blop,
        makeShapeData2(
          bounds,
          shapeType,
          radius.data(),
          smooth,
          [&](EModelShapeType type)
          {
            if (type == EModelShapeType::CONTOUR)
            {
              auto c = makeContourData2(geo);
              c->cornerSmooth = smooth;
              return c;
            }
            else if (type == EModelShapeType::POLYGON || type == EModelShapeType::STAR)
            {
              auto cp = j;
              if (pathChange(cp))
              {
                auto c = makeContourData2(cp["shape"]["subshapes"][0]["subGeometry"]);
                c->cornerSmooth = smooth;
                return c;
              }
            }
            return std::make_shared<ContourArray>();
          }));
    }
    else
    {
      const auto modelType = toModelType(klass);
      res.emplace_back(blop, dispatchObject(modelType, std::move(geo)));
    }
  }
  return res;
}
}; // namespace VGG::layer
