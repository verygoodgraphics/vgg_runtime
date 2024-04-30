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
#pragma once

#include "Layer/Model/Concept.hpp"
#include "Layer/Model/JSONModelSerde.hpp"
#include "Layer/Model/ModelUtils.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/NlohmannJSONImpl.hpp"

#include <nlohmann/json.hpp>

using namespace nlohmann;

namespace VGG::layer
{

#define M_JSON_FIELD_DEF(getter, key, type, dft) NLOHMANN_JSON_MODEL_IMPL(getter, key, type, dft)
#define M_CLASS_GETTER NLOHMANN_JSON_OBJECT_MODEL_CLASS_GETTER

struct JSONObject
{
  DECL_MODEL_OBJECT(JSONObject, JSONObject);
  const json&      j;
  EModelObjectType type;
  JSONObject(const json& j, EModelObjectType t)
    : j(j)
    , type(t)
  {
  }
  EModelObjectType getObjectType() const
  {
    return type;
  }
  std::vector<JSONObject> getChildObjects() const;
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
  JSONImageObject(const json& j)
    : JSONObject(j, EModelObjectType::IMAGE)
  {
  }
  M_JSON_FIELD_DEF(ImageBounds, "bounds", Bounds, {});
  M_JSON_FIELD_DEF(ImageGUID, "imageFileName", std::string, "");
  M_JSON_FIELD_DEF(ImageFilter, "imageFilters", ImageFilter, {});
};

struct JSONTextObject : public JSONObject
{
  DECL_MODEL_TEXT(JSONTextObject, JSONObject);
  JSONTextObject(const json& j)
    : JSONObject(j, EModelObjectType::TEXT)
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
  M_JSON_FIELD_DEF(Anchor, "anchorPoint", std::optional<Float2>, std::nullopt);
  M_JSON_FIELD_DEF(TextLineType, "textLineType", std::vector<TextLineAttr>, {});
  M_JSON_FIELD_DEF(DefaultFontAttr, "defaultFontAttr", TextStyleAttr, {});
  M_JSON_FIELD_DEF(FontAttr, "fontAttr", std::vector<TextStyleAttr>, {});
  M_JSON_FIELD_DEF(
    HorizontalAlignment,
    "horizontalAlignment",
    std::vector<ETextHorizontalAlignment>,
    {});

  std::vector<TextStyleAttr> getOverrideFontAttr() const;
};

struct JSONGroupObject : public JSONObject
{
  DECL_MODEL_GROUP(JSONGroupObject, JSONObject);
  JSONGroupObject(const json& j)
    : JSONObject(j, EModelObjectType::GROUP)
  {
  }
};

struct JSONFrameObject : public JSONObject
{
  DECL_MODEL_FRAME(JSONFrameObject, JSONObject);
  JSONFrameObject(const json& j)
    : JSONObject(j, EModelObjectType::FRAME)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONMasterObject : public JSONObject
{
  DECL_MODEL_MASTER(JSONMasterObject, JSONObject);
  JSONMasterObject(const json& j)
    : JSONObject(j, EModelObjectType::MASTER)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONPathObject : public JSONObject
{
  DECL_MODEL_PATH(JSONPathObject, JSONObject);
  JSONPathObject(const json& j)
    : JSONObject(j, EModelObjectType::PATH)
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
  JSONInstanceObject(const json& j)
    : JSONObject(j, EModelObjectType::INSTANCE)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONModelCastObject
{
  static JSONGroupObject asGroup(const JSONObject& o)
  {
    return JSONGroupObject(o.j);
  }

  static JSONFrameObject asFrame(const JSONObject& o)
  {
    return JSONFrameObject(o.j);
  }

  static JSONImageObject asImage(const JSONObject& o)
  {
    return JSONImageObject(o.j);
  }

  static JSONTextObject asText(const JSONObject& o)
  {
    return JSONTextObject(o.j);
  }

  static JSONMasterObject asMaster(const JSONObject& o)
  {
    return JSONMasterObject(o.j);
  }

  static JSONPathObject asPath(const JSONObject& o)
  {
    return JSONPathObject(o.j);
  }

  static JSONInstanceObject asInstance(const JSONObject& o)
  {
    return JSONInstanceObject(o.j);
  }
};

using JSONModelFrame = ModelPolicy<JSONFrameObject, JSONModelCastObject>;

} // namespace VGG::layer
