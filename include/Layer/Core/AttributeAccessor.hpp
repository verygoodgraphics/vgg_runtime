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
#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/Transform.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/AttributeNode.hpp"
#include "Layer/Config.hpp"

#include "Layer/ParagraphLayout.hpp" //TODO:: It's a private header

#define ATTR_MEMBER_GETTER(name, type, container)                                                  \
  type* name() const                                                                               \
  {                                                                                                \
    return container;                                                                              \
  }

namespace VGG::layer
{

class TransformAttribute;
class AlphaMaskAttribute;
class ShapeMaskAttribute;
class ObjectAttribute;
class DropShadowAttribute;
class InnerShadowAttribute;
class LayerFXAttribute;
class BackdropFXAttribute;
class ShapeAttribute;
class PaintNode;

class ParagraphItem;
class ImageItem;

class VGG_EXPORTS Accessor
{
public:
  Accessor(const Accessor&) = default;
  Accessor& operator=(const Accessor& a) = delete;
  Accessor(Accessor&&) = default;
  Accessor& operator=(Accessor&&) = delete;
  ~Accessor() = default;

  PaintNode* owner() const
  {
    return m_owner;
  }

  ATTR_DECL(Transform, const Transform&);
  ATTR_DECL(AlphaMask, const std::vector<AlphaMask>&);
  ATTR_DECL(ShapeMask, const std::vector<std::string>&);
  ATTR_DECL(DropShadows, const std::vector<DropShadow>&);
  ATTR_DECL(InnerShadows, const std::vector<InnerShadow>&);
  ATTR_DECL(Fills, const std::vector<Fill>&);
  ATTR_DECL(Borders, const std::vector<Border>&);
  ATTR_DECL(LayerBlurs, const std::vector<LayerFX>&);
  ATTR_DECL(BackgroundBlurs, const std::vector<BackgroundFX>&);

  // The following api should not be accessed
  ATTR_MEMBER_GETTER(transform, TransformAttribute, m_transformAttr);
  ATTR_MEMBER_GETTER(alphaMask, AlphaMaskAttribute, m_alphaMaskAttr);
  ATTR_MEMBER_GETTER(shapeMask, ShapeMaskAttribute, m_shapeMaskAttr);
  ATTR_MEMBER_GETTER(dropShadow, DropShadowAttribute, m_dropShadowAttr);
  ATTR_MEMBER_GETTER(innerShadow, InnerShadowAttribute, m_innerShadowAttr);
  ATTR_MEMBER_GETTER(styleObject, ObjectAttribute, m_styleObjectAttr);
  ATTR_MEMBER_GETTER(layerFX, LayerFXAttribute, m_layerFXAttr);
  ATTR_MEMBER_GETTER(backgroundBlur, BackdropFXAttribute, m_backgroundBlurAttr);

protected:
  friend class StyleItem;

  PaintNode* const            m_owner{ nullptr };
  TransformAttribute* const   m_transformAttr;
  AlphaMaskAttribute* const   m_alphaMaskAttr;
  ShapeMaskAttribute* const   m_shapeMaskAttr;
  DropShadowAttribute* const  m_dropShadowAttr;
  InnerShadowAttribute* const m_innerShadowAttr;
  ObjectAttribute* const      m_styleObjectAttr;
  LayerFXAttribute* const     m_layerFXAttr;
  BackdropFXAttribute* const  m_backgroundBlurAttr;

  Accessor(
    PaintNode*            owner,
    TransformAttribute*   transformAttr,
    AlphaMaskAttribute*   alphaMaskAttr,
    ShapeMaskAttribute*   shapemaskAttr,
    DropShadowAttribute*  dropShadowAttr,
    InnerShadowAttribute* innerShadowAttr,
    ObjectAttribute*      objectAttr,
    LayerFXAttribute*     layerPostProcessAttr,
    BackdropFXAttribute*  backgroundBlurAttr)
    : m_owner(owner)
    , m_transformAttr(transformAttr)
    , m_alphaMaskAttr(alphaMaskAttr)
    , m_shapeMaskAttr(shapemaskAttr)
    , m_dropShadowAttr(dropShadowAttr)
    , m_innerShadowAttr(innerShadowAttr)
    , m_styleObjectAttr(objectAttr)
    , m_layerFXAttr(layerPostProcessAttr)
    , m_backgroundBlurAttr(backgroundBlurAttr)
  {
  }
};

class VGG_EXPORTS ShapeItemAttibuteAccessor : public Accessor
{
public:
  ShapeItemAttibuteAccessor(const Accessor& acc, ShapeAttribute* shape)
    : Accessor(acc)
    , m_shapeAttr(shape)
  {
  }

  ShapeItemAttibuteAccessor(
    PaintNode*            owner,
    ShapeAttribute*       shapeAttr,
    TransformAttribute*   transformAttr,
    AlphaMaskAttribute*   alphaMaskAttr,
    ShapeMaskAttribute*   shapemaskAttr,
    DropShadowAttribute*  dropShadowAttr,
    InnerShadowAttribute* innerShadowAttr,
    ObjectAttribute*      objectAttr,
    LayerFXAttribute*     layerPostProcessAttr,
    BackdropFXAttribute*  backgroundBlurAttr)
    : Accessor(
        owner,
        transformAttr,
        alphaMaskAttr,
        shapemaskAttr,
        dropShadowAttr,
        innerShadowAttr,
        objectAttr,
        layerPostProcessAttr,
        backgroundBlurAttr)
    , m_shapeAttr(shapeAttr)
  {
  }
  ATTR_DECL(Shape, const VShape&);

  // The following api should not be accessed
  ATTR_MEMBER_GETTER(shape, ShapeAttribute, m_shapeAttr);

protected:
  ShapeAttribute* const m_shapeAttr;
};

class VGG_EXPORTS ParagraphItemAttributeAccessor : public Accessor
{
public:
  ParagraphItemAttributeAccessor(const Accessor& acc, ParagraphItem* paragraphObjectAttr)
    : Accessor(acc)
    , m_paraAttr(paragraphObjectAttr)
  {
  }
  ParagraphItemAttributeAccessor(
    PaintNode*            owner,
    ParagraphItem*        paragraphObjectAttr,
    TransformAttribute*   transformAttr,
    AlphaMaskAttribute*   alphaMaskAttr,
    ShapeMaskAttribute*   shapemaskAttr,
    DropShadowAttribute*  dropShadowAttr,
    InnerShadowAttribute* innerShadowAttr,
    ObjectAttribute*      objectAttr,
    LayerFXAttribute*     layerPostProcessAttr,
    BackdropFXAttribute*  backgroundBlurAttr)
    : Accessor(
        owner,
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
  void setParagraph(
    std::string                text,
    std::vector<TextStyleAttr> style,
    std::vector<ParagraphAttr> parStyle);
  ATTR_DECL(ParagraphBounds, const Bounds&);
  ATTR_DECL(Anchor, glm::vec2);
  ATTR_DECL(TextLayoutMode, ETextLayoutMode);
  ATTR_DECL(VerticalAlignment, const ETextVerticalAlignment&);

  // The following api should not be accessed
  ATTR_MEMBER_GETTER(paragraph, ParagraphItem, m_paraAttr);

protected:
  ParagraphItem* const m_paraAttr;
};

class VGG_EXPORTS ImageItemAttribtueAccessor : public Accessor
{
public:
  ImageItemAttribtueAccessor(const Accessor& acc, ImageItem* image)
    : Accessor(acc)
    , m_imageAttr(image)
  {
  }
  ATTR_DECL(ImageGUID, const std::string&);
  ATTR_DECL(ImageFilter, const ImageFilter&);
  ATTR_DECL(ImageBounds, const Bounds&);

  ATTR_MEMBER_GETTER(image, ImageItem, m_imageAttr);

protected:
  ImageItem* const m_imageAttr;
};

#undef ATTR_MEMBER_GETTER
} // namespace VGG::layer
