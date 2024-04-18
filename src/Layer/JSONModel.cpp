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
#include "Layer/Model/Concept.hpp"
#include "Layer/Model/ModelUtils.hpp"

#include <vector>

namespace
{

using namespace VGG;
using namespace VGG::layer;

inline ContourPtr makeContourData(const json& j)
{
  const auto&   points = getOrDefault(j, "points");
  BezierContour contour(points.size());
  contour.closed = j.value("closed", false);
  for (const auto& e : points)
  {
    contour.emplace_back(
      getOptional<glm::vec2>(e, "point").value_or(glm::vec2{ 0, 0 }),
      getOptional<float>(e, "radius").value_or(0.0),
      getOptional<glm::vec2>(e, "curveFrom"),
      getOptional<glm::vec2>(e, "curveTo"),
      getOptional<int>(e, "cornerStyle"));
  }
  auto ptr = std::make_shared<BezierContour>(contour);
  if (!ptr->closed && !ptr->empty())
  {
    ptr->back().radius = 0;
    ptr->front().radius = 0;
  }
  return ptr;
}

JSONObject dispatchObject(EModelObjectType modelType, const json& j)
{
  switch (modelType)
  {
    case EModelObjectType::FRAME:
      return JSONFrameObject(j);
    case EModelObjectType::GROUP:
      return JSONGroupObject(j);
    case EModelObjectType::PATH:
      return JSONPathObject(j);
    case EModelObjectType::TEXT:
      return JSONTextObject(j);
    case EModelObjectType::MASTER:
      return JSONMasterObject(j);
    case EModelObjectType::IMAGE:
      return JSONImageObject(j);
    case EModelObjectType::OBJECT:
      return JSONObject(j, EModelObjectType::OBJECT);
    case EModelObjectType::INSTANCE:
      return JSONInstanceObject(j);
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
  if (j.find("childObjects") == j.end())
    return std::vector<JSONObject>();
  const auto&             childObjects = j["childObjects"];
  std::vector<JSONObject> objects;
  objects.reserve(childObjects.size());
  for (const auto& j : childObjects)
  {
    const auto klass = j.value("class", "");
    auto       modelType = toModelType(klass);
    objects.emplace_back(dispatchObject(modelType, j));
  }
  return objects;
}

std::vector<SubShape<JSONObject>> JSONPathObject::getShapes() const
{
  std::vector<SubShape<JSONObject>> res;
  if (j.find("shape") == j.end() || j["shape"].find("subshapes") == j["shape"].end())
    return res;
  const auto& shapes = j["shape"]["subshapes"];
  const auto  bounds = getBounds();
  const auto  smooth = getCornerSmoothing();
  for (const auto& subshape : shapes)
  {
    if (subshape.find("subGeometry") == subshape.end())
      continue;
    const auto            blop = subshape.value("booleanOperation", EBoolOp::BO_NONE);
    const auto&           geo = subshape["subGeometry"];
    const auto            klass = geo.value("class", "");
    const EModelShapeType shapeType = toShapeType(klass);
    if (shapeType != EModelShapeType::UNKNOWN)
    {
      Float4 radius = { 0, 0, 0, 0 };
      int    countPoint = 0;
      if (shapeType == EModelShapeType::POLYGON)
      {
        radius[0] = geo.value("radius", 0.f);
        countPoint = geo.value("pointCount", 0);
      }
      else if (shapeType == EModelShapeType::STAR)
      {
        radius[0] = geo.value("radius", 0.f);
        radius[1] = geo.value("ratio", 0.f);
        countPoint = geo.value("pointCount", 0);
      }
      else if (shapeType == EModelShapeType::RECTANGLE)
      {
        radius = geo.value("radius", Float4{ 0.f, 0.f, 0.f, 0.f });
      }
      res.emplace_back(
        blop,
        makeShapeData(
          bounds,
          shapeType,
          radius.data(),
          smooth,
          countPoint,
          [&](EModelShapeType type)
          {
            if (type == EModelShapeType::CONTOUR)
            {
              auto c = makeContourData(geo);
              c->cornerSmooth = smooth;
              return c;
            }
            return std::make_shared<BezierContour>(4);
          }));
    }
    else
    {
      const auto modelType = toModelType(klass);
      res.emplace_back(blop, dispatchObject(modelType, geo));
    }
  }
  return res;
}

std::vector<TextStyleAttr> JSONTextObject::getOverrideFontAttr() const
{
  std::vector<TextStyleAttr> textStyle;
  static const auto          s_defaultAttr = json::parse(DEFAULT_FONT_ATTR);
  auto                       defaultAttr = s_defaultAttr;
  defaultAttr.update(j.value("defaultFontAttr", json::object()), true);
  auto fontAttr = j.value("fontAttr", std::vector<json>{});
  for (auto& att : fontAttr)
  {
    auto json = defaultAttr;
    json.update(att, true);
    if (auto it = json.find("fills"); it == json.end())
    {
      json["fills"] =
        j.value("style", nlohmann::json{}).value("fills", std::vector<nlohmann::json>());
    }
    textStyle.push_back(json);
  }
  return textStyle;
}

}; // namespace VGG::layer
