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

#include <vector>

namespace
{

using namespace VGG;
using namespace VGG::layer;

inline ContourPtr makeContourData2(const json& j)
{
  const auto&  points = getOrDefault(j, "points");
  ContourArray contour(points.size());
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
        makeShapeData2(
          bounds,
          shapeType,
          radius.data(),
          smooth,
          countPoint,
          [&](EModelShapeType type)
          {
            if (type == EModelShapeType::CONTOUR)
            {
              auto c = makeContourData2(geo);
              c->cornerSmooth = smooth;
              return c;
            }
            return std::make_shared<ContourArray>(4);
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

std::vector<TextStyleAttr> JSONTextObject::getOverrideFontAttr() const
{
  std::vector<TextStyleAttr> textStyle;
  static const auto          s_defaultAttr = R"({
        "length":0,
        "name":"Fira Sans",
        "subFamilyName":"",
        "size":14,
        "fontVariations":[],
        "postScript":"",
        "kerning":true,
        "letterSpacingValue":0,
        "letterSpacingUnit":0,
        "lineSpacingValue":0,
        "lineSpacingUnit":0,
        "fillUseType":0,
        "underline":0,
        "linethrough":false,
        "fontVariantCaps":0,
        "textCase":0,
        "baselineShift":0,
        "baseline":0,
        "horizontalScale":1,
        "verticalScale":1,
        "proportionalSpacing":0,
        "rotate":0,
        "textParagraph":{}
    })"_json;
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
