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

#include "Layer/MaskAttribute.hpp"
#include "Layer/ObjectAttribute.hpp"
#include "Layer/StyleItem.hpp"
#include "Layer/ShadowAttribute.hpp"
#include "Layer/TransformAttribute.hpp"
#include "Layer/ImageGraphicItem.hpp"
#include "Layer/ParagraphGraphicItem.hpp"

#include "Layer/Core/AttributeAccessor.hpp"
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/ParagraphGraphicItem.hpp"
#include "Layer/Core/PaintNode.hpp"

#define MODEL_DEF(classname, name, type)                                                           \
  void classname::set##name(const std::remove_cvref_t<type>& v)                                    \
  {                                                                                                \
    owner()->set##name(v);                                                                         \
  }                                                                                                \
  void classname::set##name(std::remove_cvref_t<type>&& v)                                         \
  {                                                                                                \
    owner()->set##name(std::move(v));                                                              \
  }                                                                                                \
  type classname::get##name() const                                                                \
  {                                                                                                \
    return owner()->get##name();                                                                   \
  }

namespace VGG::layer
{

ATTR_DEF(Accessor, Transform, const Transform&, Transform, m_transformAttr);
ATTR_DEF(Accessor, AlphaMask, const std::vector<AlphaMask>&, AlphaMasks, m_alphaMaskAttr);
ATTR_DEF(Accessor, ShapeMask, const std::vector<std::string>&, MaskID, m_shapeMaskAttr);
ATTR_DEF(Accessor, DropShadows, const std::vector<DropShadow>&, DropShadowStyle, m_dropShadowAttr);
ATTR_DEF(
  Accessor,
  InnerShadows,
  const std::vector<InnerShadow>&,
  InnerShadowStyle,
  m_innerShadowAttr);
MODEL_DEF(Accessor, Fills, const std::vector<Fill>&);
MODEL_DEF(Accessor, Borders, const std::vector<Border>&);
ATTR_DEF(Accessor, LayerBlurs, const std::vector<LayerFX>&, LayerBlur, m_layerFXAttr);
ATTR_DEF(
  Accessor,
  BackgroundBlurs,
  const std::vector<BackgroundFX>&,
  BackgroundBlur,
  m_backgroundBlurAttr);

ATTR_DEF(
  ParagraphItemAttributeAccessor,
  ParagraphBounds,
  const Bounds&,
  ParagraphBounds,
  m_paraAttr);
ATTR_DEF(ParagraphItemAttributeAccessor, Anchor, glm::vec2, Anchor, m_paraAttr);
ATTR_DEF(ParagraphItemAttributeAccessor, TextLayoutMode, ETextLayoutMode, FrameMode, m_paraAttr);

ATTR_DEF(
  ParagraphItemAttributeAccessor,
  VerticalAlignment,
  const ETextVerticalAlignment&,
  VerticalAlignment,
  m_paraAttr);

void ParagraphItemAttributeAccessor::setParagraph(
  std::string                text,
  std::vector<TextStyleAttr> style,
  std::vector<ParagraphAttr> parStyle)
{
  ASSERT(m_paraAttr);
  m_paraAttr->setParagraph(std::move(text), std::move(style), std::move(parStyle));
}

ATTR_DEF(ImageItemAttribtueAccessor, ImageGUID, const std::string&, ImageGUID, m_imageAttr);
ATTR_DEF(ImageItemAttribtueAccessor, ImageFilter, const ImageFilter&, ImageFilter, m_imageAttr);
ATTR_DEF(ImageItemAttribtueAccessor, ImageBounds, const Bounds&, ImageBounds, m_imageAttr);

} // namespace VGG::layer
