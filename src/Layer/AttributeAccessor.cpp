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

#include "AttributeAccessor.hpp"

#include "Layer/MaskAttribute.hpp"
#include "Layer/ObjectAttribute.hpp"
#include "Layer/ShadowAttribute.hpp"
#include "Layer/TransformAttribute.hpp"
#include "Layer/ImageGraphicItem.hpp"
#include "Layer/ParagraphGraphicItem.hpp"

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/ParagraphGraphicItem.hpp"

#define ATTR_DEF(classname, name, type, internalname, container)                                   \
  void classname::set##name(const type& v)                                                         \
  {                                                                                                \
    ASSERT(container != nullptr);                                                                  \
    container->set##internalname(v);                                                               \
  }                                                                                                \
                                                                                                   \
  void classname::set##name(type&& v)                                                              \
  {                                                                                                \
    ASSERT(container != nullptr);                                                                  \
    container->set##internalname(std::move(v));                                                    \
  }

namespace VGG::layer
{

ATTR_DEF(Accessor, Transform, Transform, Transform, m_transformAttr);
ATTR_DEF(Accessor, AlphaMask, std::vector<AlphaMask>, AlphaMasks, m_alphaMaskAttr);
ATTR_DEF(Accessor, ShapeMask, std::vector<std::string>, MaskID, m_shapeMaskAttr);

ATTR_DEF(Accessor, DropShadows, std::vector<DropShadow>, DropShadowStyle, m_dropShadowAttr);
ATTR_DEF(Accessor, InnerShadows, std::vector<InnerShadow>, InnerShadowStyle, m_innerShadowAttr);
ATTR_DEF(Accessor, Fills, std::vector<Fill>, FillStyle, m_styleObjectAttr);
ATTR_DEF(Accessor, Borders, std::vector<Border>, BorderStyle, m_styleObjectAttr);
ATTR_DEF(Accessor, LayerBlurs, std::vector<LayerFX>, LayerBlur, m_layerFXAttr);
ATTR_DEF(
  Accessor,
  BackgroundBlurs,
  std::vector<BackgroundFX>,
  BackgroundBlur,
  m_backgroundBlurAttr);

ATTR_DEF(ShapeGraphicItemAttibuteAccessor, Shape, VShape, Shape, m_shapeAttr);
ATTR_DEF(ParagraphItemAccessor, ParagraphBound, Bounds, ParagraphBound, m_paraAttr);
ATTR_DEF(ParagraphItemAccessor, Anchor, glm::vec2, Anchor, m_paraAttr);
ATTR_DEF(ParagraphItemAccessor, TextLayoutMode, ETextLayoutMode, FrameMode, m_paraAttr);

ATTR_DEF(
  ParagraphItemAccessor,
  VerticalAlignment,
  ETextVerticalAlignment,
  VerticalAlignment,
  m_paraAttr);

void ParagraphItemAccessor::setParagraph(
  std::string                text,
  std::vector<TextStyleAttr> style,
  std::vector<ParagraphAttr> parStyle)
{
  ASSERT(m_paraAttr);
  m_paraAttr->setParagraph(std::move(text), std::move(style), std::move(parStyle));
}

ATTR_DEF(ImageItemAttribtueAccessor, ImageGUID, std::string, ImageGUID, m_imageAttr);
ATTR_DEF(ImageItemAttribtueAccessor, ImageFilter, ImageFilter, ImageFilter, m_imageAttr);
ATTR_DEF(ImageItemAttribtueAccessor, ImageBounds, Bounds, ImageBounds, m_imageAttr);

#undef ATTR_DEF

} // namespace VGG::layer
