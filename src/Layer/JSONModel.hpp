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
#include <nlohmann/json.hpp>

using namespace nlohmann;

namespace VGG::layer
{

#define M_DEF(getter, key, type, dft)                                                              \
  type get##getter() const                                                                         \
  {                                                                                                \
    return j.value<type>(key, dft);                                                                \
  }

struct JSONObject
{
  json j;
  JSONObject(json j)
    : j(std::move(j))
  {
  }
  EObjectType getObjectType() const
  {
    return EObjectType{};
  };

  M_DEF(Name, "name", std::string, "");
  M_DEF(Id, "id", std::string, "");
  M_DEF(Bounds, "bounds", Bounds, {});
  M_DEF(Matrix, "matrix", glm::mat3, { 1.f });
  M_DEF(Visible, "visible", bool, true);
  M_DEF(Overflow, "overflow", EOverflow, EOverflow::OF_VISIBLE);
  M_DEF(Style, "style", Style, Style{});
  M_DEF(ContextSetting, "contextSettings", ContextSetting, {});
  M_DEF(CornerSmoothing, "cornerSmoothin", float, 0.f);
  M_DEF(MaskType, "maskType", EMaskType, EMaskType::MT_NONE);
  M_DEF(MaskShowType, "maskShowType", EMaskShowType, EMaskShowType::MST_BOUND);
  M_DEF(AlphaMask, "alphaMaskBy", std::vector<AlphaMask>, {});
  M_DEF(ShapeMask, "outlineMaskBy", std::vector<std::string>, {});

  std::vector<JSONObject> getChildObjects();
};

struct JSONImageObject : public JSONObject
{
  JSONImageObject(json j)
    : JSONObject(std::move(j))
  {
  }
  M_DEF(ImageBounds, "bounds", Bounds, {});
  M_DEF(ImageGUID, "imageFillName", std::string, "");
  M_DEF(ImageFilter, "imageFilters", ImageFilter, {});
};

struct JSONTextObject : public JSONObject
{
  JSONTextObject(json j)
    : JSONObject(std::move(j))
  {
  }

  M_DEF(Text, "content", std::string, "");
  M_DEF(TextBounds, "bounds", Bounds, {});
  M_DEF(LayoutMode, "frameMode", ETextLayoutMode, ETextLayoutMode::TL_FIXED);
  M_DEF(
    VerticalAlignment,
    "verticalAlignment",
    ETextVerticalAlignment,
    ETextVerticalAlignment::VA_TOP);

  M_DEF(Anchor, "anchorPoint", Float2, (Float2{ 0.f, 0.f }));
};

struct JSONGroupObject : public JSONObject
{
  JSONGroupObject(json j)
    : JSONObject(std::move(j))
  {
  }
};

struct JSONFrameObject : public JSONObject
{
  JSONFrameObject(json j)
    : JSONObject(std::move(j))
  {
  }
  M_DEF(Radius, "radius", Float4, (Float4{ 0.f, 0.f, 0.f, 0.f }));
};

std::vector<JSONObject> JSONObject::getChildObjects()
{
  return {};
}

} // namespace VGG::layer
