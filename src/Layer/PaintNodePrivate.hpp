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
#include "Layer/Core/VType.hpp"
#include "Utility/HelperMacro.hpp"
#include "Layer/Core/PaintNode.hpp"
namespace VGG
{

class PaintNode__pImpl // NOLINT
{
  VGG_DECL_API(PaintNode);

public:
  Bound2 bound;
  glm::mat3 transform{ 1.0 };
  std::string guid{};
  std::vector<std::string> maskedBy{};
  Mask outlineMask;
  EMaskType maskType{ MT_None };
  EMaskShowType maskShowType{ MST_Invisible };
  EBoolOp clipOperator{ BO_None };
  EOverflow overflow{ OF_Hidden };
  EWindingType windingRule{ WR_EvenOdd };
  Style style;
  ContextSetting contextSetting;
  ObjectType type;
  bool visible{ true };

  ContourPtr contour;
  PaintOption paintOption;
  ContourOption maskOption;

  std::optional<SkPath> path;
  std::optional<SkPath> mask;

  PaintNode__pImpl(PaintNode* api, ObjectType type)
    : q_ptr(api)
    , type(type)
  {
  }
  PaintNode__pImpl(const PaintNode__pImpl& other)
  {
    this->operator=(other);
  }
  PaintNode__pImpl& operator=(const PaintNode__pImpl& other)
  {
    bound = other.bound;
    transform = other.transform;
    guid = other.guid + "_Copy";
    maskedBy = other.maskedBy;
    outlineMask = other.outlineMask;
    maskType = other.maskType;
    clipOperator = other.clipOperator;
    overflow = other.overflow;
    windingRule = other.windingRule;
    style = other.style;
    contextSetting = other.contextSetting;
    type = other.type;
    visible = other.visible;
    contour = other.contour;
    paintOption = other.paintOption;
    maskOption = other.maskOption;
    return *this;
  }

  PaintNode__pImpl(PaintNode__pImpl&&) noexcept = default;
  PaintNode__pImpl& operator=(PaintNode__pImpl&&) noexcept = default;
};
} // namespace VGG
