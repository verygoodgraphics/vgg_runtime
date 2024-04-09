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
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/AttrSerde.hpp"
#include "Layer/NlohmannJSONImpl.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/PathPatch.h"

#include <nlohmann/json.hpp>

using namespace nlohmann;

namespace VGG::layer
{

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

inline ShapeData makeShapeData2(
  const json&   j,
  const json&   parent,
  const Bounds& bounds,
  float         cornerSmoothing)
{
  const auto klass = j.value("class", "");
  if (klass == "contour")
  {
    auto c = makeContourData2(j);
    c->cornerSmooth = cornerSmoothing;
    return c;
  }
  else if (klass == "rectangle")
  {
    const auto           rect = toSkRect(bounds);
    std::array<float, 4> radius = j.value("radius", std::array<float, 4>{ 0, 0, 0, 0 });
    auto                 s = makeShape(radius, rect, cornerSmoothing);
    return std::visit([&](auto&& arg) { return ShapeData(arg); }, s);
  }
  else if (klass == "ellipse")
  {
    Ellipse oval;
    oval.rect = toSkRect(bounds);
    return oval;
  }
  else if (klass == "polygon" || klass == "star")
  {
    auto cp = parent;
    if (pathChange(cp))
    {
      auto c = makeContourData2(cp["shape"]["subshapes"][0]["subGeometry"]);
      c->cornerSmooth = cornerSmoothing;
      return c;
    }
  }
  return ShapeData();
}

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

inline JSONObject dispatchObject(std::string_view klass, json j)
{
  if (klass == "object")
  {
    return JSONObject(std::move(j), EModelObjectType::OBJECT);
  }
  else if (klass == "group")
  {
    return JSONGroupObject(std::move(j));
  }
  else if (klass == "frame")
  {
    return JSONFrameObject(std::move(j));
  }
  else if (klass == "path")
  {
    return JSONPathObject(std::move(j));
  }
  else if (klass == "image")
  {
    return JSONImageObject(std::move(j));
  }
  else if (klass == "text")
  {
    return JSONTextObject(std::move(j));
  }
  else if (klass == "symbolMaster")
  {
    return JSONMasterObject(std::move(j));
  }
  else if (klass == "symbolInstance")
  {
    return JSONInstanceObject(std::move(j));
  }
  else
  {
    return JSONObject(j, EModelObjectType::UNKNOWN);
  }
}

inline std::vector<JSONObject> JSONObject::getChildObjects() const
{
  std::vector<JSONObject> objects;
  const auto              childObjects = getOrDefault(j, "childObjects");
  for (auto& j : childObjects)
  {
    auto klass = j.value("class", "");
    objects.emplace_back(dispatchObject(klass, std::move(j)));
  }
  return objects;
}

inline std::vector<SubShape<JSONObject>> JSONPathObject::getShapes() const
{
  std::vector<SubShape<JSONObject>> res;
  const auto shapes = j.value("shape", json{}).value("subshapes", std::vector<json>());
  for (const auto& subshape : shapes)
  {
    const auto blop = subshape.value("booleanOperation", EBoolOp::BO_NONE);
    const auto geo = subshape.value("subGeometry", nlohmann::json{});
    const auto klass = geo.value("class", "");
    if (
      klass == "contour" || klass == "rectangle" || klass == "ellipse" || klass == "polygon" ||
      klass == "star")
    {
      res.emplace_back(blop, makeShapeData2(std::move(geo), j, getBounds(), getCornerSmoothing()));
    }
    else
    {
      res.emplace_back(blop, dispatchObject(klass, std::move(geo)));
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
