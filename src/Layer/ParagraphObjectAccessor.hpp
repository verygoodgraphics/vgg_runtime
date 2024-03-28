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
#include <type_traits>
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/ParagraphObjectAttribute.hpp"
#include "ShapeAttribute.hpp"
#include "LayerAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "MaskAttribute.hpp"
#include "TransformAttribute.hpp"
#include "AttributeAccessor.hpp"

class ParagraphAttributeAccessor : public Accessor
{
public:
#define ATTR_MEMBER_GETTER(name, type, container)                                                  \
  type* name() const                                                                               \
  {                                                                                                \
    return container;                                                                              \
  }
  ATTR_MEMBER_GETTER(paragraph, ParagraphObjectAttribute, m_paraAttr);
  ParagraphAttributeAccessor(const Accessor& acc, ParagraphObjectAttribute* paragraphObjectAttr)
    : Accessor(acc)
    , m_paraAttr(paragraphObjectAttr)
  {
  }
#undef ATTR_MEMBER_GETTER
  ParagraphAttributeAccessor(
    ParagraphObjectAttribute* paragraphObjectAttr,
    TransformAttribute*       transformAttr,
    AlphaMaskAttribute*       alphaMaskAttr,
    ShapeMaskAttribute*       shapemaskAttr,
    DropShadowAttribute*      dropShadowAttr,
    InnerShadowAttribute*     innerShadowAttr,
    ObjectAttribute*          objectAttr,
    LayerFXAttribute*         layerPostProcessAttr,
    BackgroundBlurAttribute*  backgroundBlurAttr)
    : Accessor(
        transformAttr,
        alphaMaskAttr,
        shapemaskAttr,
        dropShadowAttr,
        innerShadowAttr,
        objectAttr,
        layerPostProcessAttr,
        backgroundBlurAttr)
    , m_paraAttr(paragraphObjectAttr)
  {
  }

protected:
  ParagraphObjectAttribute* const m_paraAttr;
};
