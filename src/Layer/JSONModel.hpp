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

#include "DocConcept.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/AttrSerde.hpp"
#include "Layer/NlohmannJSONImpl.hpp"
#include <type_traits>

using namespace nlohmann;

namespace VGG::layer
{

#define M_JSON_FIELD_DEF(getter, key, type, dft) NLOHMANN_JSON_MODEL_IMPL(getter, key, type, dft)
#define M_CLASS_GETTER NLOHMANN_JSON_OBJECT_MODEL_CLASS_GETTER

#define M_OBJECT_TYPE_DEF                                                                          \
  EModelObjectType getObjectType() const                                                           \
  {                                                                                                \
    auto klass = M_CLASS_GETTER;                                                                   \
    if (klass == "group")                                                                          \
      return EModelObjectType::GROUP;                                                              \
    if (klass == "frame")                                                                          \
      return EModelObjectType::FRAME;                                                              \
    if (klass == "path")                                                                           \
      return EModelObjectType::PATH;                                                               \
    if (klass == "image")                                                                          \
      return EModelObjectType::IMAGE;                                                              \
    if (klass == "text")                                                                           \
      return EModelObjectType::TEXT;                                                               \
    if (klass == "contour")                                                                        \
      return EModelObjectType::CONTOUR;                                                            \
    if (klass == "master")                                                                         \
      return EModelObjectType::MASTER;                                                             \
    if (klass == "instance")                                                                       \
      return EModelObjectType::INSTANCE;                                                           \
    return EModelObjectType::UNKNOWN;                                                              \
  }

struct JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONObject;
  json             j;
  EModelObjectType type;
  JSONObject(json j, EModelObjectType type)
    : j(std::move(j))
    , type(type)
  {
  }
  std::vector<JSONObject> getChildObjects() const;
  EModelObjectType        getObjectType() const
  {
    return type;
  }
  M_JSON_FIELD_DEF(Name, "name", std::string, "");
  M_JSON_FIELD_DEF(Id, "id", std::string, "");
  M_JSON_FIELD_DEF(Bounds, "bounds", Bounds, {});
  M_JSON_FIELD_DEF(Matrix, "matrix", glm::mat3, { 1.f });
  M_JSON_FIELD_DEF(Visible, "visible", bool, true);
  M_JSON_FIELD_DEF(Overflow, "overflow", EOverflow, EOverflow::OF_VISIBLE);
  M_JSON_FIELD_DEF(Style, "style", Style, Style{});
  M_JSON_FIELD_DEF(ContextSetting, "contextSettings", ContextSetting, {});
  M_JSON_FIELD_DEF(CornerSmoothing, "cornerSmoothin", float, 0.f);
  M_JSON_FIELD_DEF(MaskType, "maskType", EMaskType, EMaskType::MT_NONE);
  M_JSON_FIELD_DEF(MaskShowType, "maskShowType", EMaskShowType, EMaskShowType::MST_BOUNDS);
  M_JSON_FIELD_DEF(ShapeMask, "outlineMaskBy", std::vector<std::string>, {});
  M_JSON_FIELD_DEF(AlphaMask, "alphaMaskBy", std::vector<AlphaMask>, {});
};

struct JSONImageObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONImageObject;
  JSONImageObject(json j)
    : JSONObject(std::move(j), EModelObjectType::IMAGE)
  {
  }
  M_JSON_FIELD_DEF(ImageBounds, "bounds", Bounds, {});
  M_JSON_FIELD_DEF(ImageGUID, "imageFillName", std::string, "");
  M_JSON_FIELD_DEF(ImageFilter, "imageFilters", ImageFilter, {});
};

struct JSONTextObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONTextObject;
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
  M_JSON_FIELD_DEF(
    HorizontalAlignment,
    "horizontalAlignment",
    std::vector<ETextHorizontalAlignment>,
    {});
};

struct JSONGroupObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONGroupObject;
  JSONGroupObject(json j)
    : JSONObject(std::move(j), EModelObjectType::GROUP)
  {
  }
};

struct JSONFrameObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONFrameObject;
  JSONFrameObject(json j)
    : JSONObject(std::move(j), EModelObjectType::FRAME)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONMasterObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONMasterObject;
  JSONMasterObject(json j)
    : JSONObject(std::move(j), EModelObjectType::MASTER)
  {
  }
  M_JSON_FIELD_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

struct JSONPathObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONPathObject;
  JSONPathObject(json j)
    : JSONObject(std::move(j), EModelObjectType::PATH)
  {
  }
  EWindingType getWindingType() const
  {
    return EWindingType::WR_EVEN_ODD;
  }

  std::vector<SubShape<JSONObject>> getShapes() const;
};

struct JSONInstanceObject : public JSONObject
{
  using BaseType = JSONObject;
  using Self = JSONInstanceObject;
  JSONInstanceObject(json j)
    : JSONObject(std::move(j), EModelObjectType::INSTANCE)
  {
  }
};

std::vector<JSONObject> JSONObject::getChildObjects() const
{
  std::vector<JSONObject> objects;
  const auto              childObjects = getOrDefault(j, "childObjects");
  for (auto& j : childObjects)
  {
    switch (getObjectType())
    {
      case EModelObjectType::GROUP:
        objects.emplace_back(JSONGroupObject(std::move(j)));
        break;
      case EModelObjectType::FRAME:
        objects.emplace_back(JSONFrameObject(std::move(j)));
        break;
      case EModelObjectType::PATH:
        objects.emplace_back(JSONPathObject(std::move(j)));
        break;
      case EModelObjectType::IMAGE:
        objects.emplace_back(JSONImageObject(std::move(j)));
        break;
      case EModelObjectType::TEXT:
        objects.emplace_back(JSONTextObject(std::move(j)));
        break;
      case EModelObjectType::CONTOUR:
        DEBUG("not implemented");
        break;
      case EModelObjectType::MASTER:
        objects.emplace_back(JSONMasterObject(std::move(j)));
        break;
      case EModelObjectType::INSTANCE:
        objects.emplace_back(JSONInstanceObject(std::move(j)));
        break;
      case EModelObjectType::UNKNOWN:
        DEBUG("unknown object type");
        break;
    }
  }
  return objects;
}

std::vector<SubShape<JSONObject>> JSONPathObject::getShapes() const
{
  return {};
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

} // namespace VGG::layer
