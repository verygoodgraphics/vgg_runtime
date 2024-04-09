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

#include "Layer/Model/Concept.hpp"
#include "Layer/Model/ModelUtils.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/AttrSerde.hpp"
#include "Layer/NlohmannJSONImpl.hpp"
#include "Layer/VSkia.hpp"

#include <nlohmann/json.hpp>

using namespace nlohmann;

namespace VGG::layer
{

#define M_JSON_FIELD_DEF(getter, key, type, dft) NLOHMANN_JSON_MODEL_IMPL(getter, key, type, dft)
#define M_CLASS_GETTER NLOHMANN_JSON_OBJECT_MODEL_CLASS_GETTER

struct JSONObject
{
  DECL_MODEL_OBJECT(JSONObject, JSONObject);
  json             j;
  EModelObjectType type;
  JSONObject(json j, EModelObjectType t)
    : j(std::move(j))
    , type(t)
  {
  }
  EModelObjectType getObjectType() const
  {
    return type;
  }
  std::vector<JSONObject> getChildObjects() const;
  M_JSON_FIELD_DEF(ObjectTypeString, "class", std::string, "");
  M_JSON_FIELD_DEF(Name, "name", std::string, "");
  M_JSON_FIELD_DEF(Id, "id", std::string, "");
  M_JSON_FIELD_DEF(Bounds, "bounds", Bounds, {});
  M_JSON_FIELD_DEF(Matrix, "matrix", glm::mat3, { 1.f });
  M_JSON_FIELD_DEF(Visible, "visible", bool, true);
  M_JSON_FIELD_DEF(Overflow, "overflow", EOverflow, EOverflow::OF_VISIBLE);
  M_JSON_FIELD_DEF(Style, "style", Style, Style{});
  M_JSON_FIELD_DEF(ContextSetting, "contextSettings", ContextSetting, {});
  M_JSON_FIELD_DEF(CornerSmoothing, "cornerSmoothing", float, 0.f);
  M_JSON_FIELD_DEF(MaskType, "maskType", EMaskType, EMaskType::MT_NONE);
  M_JSON_FIELD_DEF(MaskShowType, "maskShowType", EMaskShowType, EMaskShowType::MST_BOUNDS);
  M_JSON_FIELD_DEF(ShapeMask, "outlineMaskBy", std::vector<std::string>, {});
  M_JSON_FIELD_DEF(AlphaMask, "alphaMaskBy", std::vector<AlphaMask>, {});
};

struct JSONImageObject : public JSONObject
{
  DECL_MODEL_IMAGE(JSONImageObject, JSONObject);
  JSONImageObject(json j)
    : JSONObject(std::move(j), EModelObjectType::IMAGE)
  {
  }
  M_JSON_FIELD_DEF(ImageBounds, "bounds", Bounds, {});
  M_JSON_FIELD_DEF(ImageGUID, "imageFileName", std::string, "");
  M_JSON_FIELD_DEF(ImageFilter, "imageFilters", ImageFilter, {});
};

struct JSONTextObject : public JSONObject
{
  DECL_MODEL_TEXT(JSONTextObject, JSONObject);
  JSONTextObject(json j)
    : JSONObject(std::move(j), EModelObjectType::TEXT)
  {
  }

  M_JSON_FIELD_DEF(Text, "content", std::string, "");
  M_JSON_FIELD_DEF(TextBounds, "bounds", Bounds, {});
  M_JSON_FIELD_DEF(LayoutMode, "frameMode", ETextLayoutMode, ETextLayoutMode::TL_FIXED);
  M_JSON_FIELD_DEF(
    VerticalAlignment,
    "verticalAlignment",
    ETextVerticalAlignment,
    ETextVerticalAlignment::VA_TOP);
  M_JSON_FIELD_DEF(Anchor, "anchorPoint", std::optional<Float2>, (std::nullopt));
  M_JSON_FIELD_DEF(TextLineType, "textLineType", std::vector<TextLineAttr>, {});
  M_JSON_FIELD_DEF(DefaultFontAttr, "defaultFontAttr", TextStyleAttr, {});
  M_JSON_FIELD_DEF(FontAttr, "fontAttr", std::vector<TextStyleAttr>, {});
  M_JSON_FIELD_DEF(
    HorizontalAlignment,
    "horizontalAlignment",
    std::vector<ETextHorizontalAlignment>,
    {});
};

struct JSONGroupObject : public JSONObject
{
  DECL_MODEL_GROUP(JSONGroupObject, JSONObject);
  JSONGroupObject(json j)
    : JSONObject(std::move(j), EModelObjectType::GROUP)
  {
  }
};

struct JSONFrameObject : public JSONObject
{
  DECL_MODEL_FRAME(JSONFrameObject, JSONObject);
  JSONFrameObject(json j)
    : JSONObject(std::move(j), EModelObjectType::FRAME)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONMasterObject : public JSONObject
{
  DECL_MODEL_MASTER(JSONMasterObject, JSONObject);
  JSONMasterObject(json j)
    : JSONObject(std::move(j), EModelObjectType::MASTER)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONPathObject : public JSONObject
{
  DECL_MODEL_PATH(JSONPathObject, JSONObject);
  JSONPathObject(json j)
    : JSONObject(std::move(j), EModelObjectType::PATH)
  {
  }
  EWindingType getWindingType() const
  {
    return j.value("shape", json{}).value("windingRule", EWindingType::WR_EVEN_ODD);
  }

  std::vector<SubShape<JSONObject>> getShapes() const;
};

struct JSONInstanceObject : public JSONObject
{
  DECL_MODEL_INSTANCE(JSONInstanceObject, JSONObject);
  JSONInstanceObject(json j)
    : JSONObject(std::move(j), EModelObjectType::INSTANCE)
  {
  }
};

inline JSONObject dispatchObject(EModelObjectType modelType, json j)
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

inline ContourPtr makeContourData2(const json& j)
{
  Contour contour;
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
  auto ptr = std::make_shared<Contour>(contour);
  if (!ptr->closed && !ptr->empty())
  {
    ptr->back().radius = 0;
    ptr->front().radius = 0;
  }
  return ptr;
}

inline std::vector<JSONObject> JSONObject::getChildObjects() const
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

inline std::vector<SubShape<JSONObject>> JSONPathObject::getShapes() const
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
            return std::make_shared<Contour>();
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

struct JSONModelCastObject
{
  static JSONGroupObject asGroup(const JSONObject& o)
  {
    return JSONGroupObject(o.j);
  }

  static JSONFrameObject asFrame(JSONObject o)
  {
    return JSONFrameObject(o.j);
  }

  static JSONImageObject asImage(JSONObject o)
  {
    return JSONImageObject(o.j);
  }

  static JSONTextObject asText(JSONObject o)
  {
    return JSONTextObject(o.j);
  }

  static JSONMasterObject asMaster(JSONObject o)
  {
    return JSONMasterObject(o.j);
  }

  static JSONPathObject asPath(JSONObject o)
  {
    return JSONPathObject(o.j);
  }

  static JSONInstanceObject asInstance(JSONObject o)
  {
    return JSONInstanceObject(o.j);
  }
};

using JSONModelFrame = ModelPolicy<JSONFrameObject, JSONModelCastObject>;

} // namespace VGG::layer
