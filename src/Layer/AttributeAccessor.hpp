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
#include "Layer/ImageRenderObjectAttribute.hpp"
#include "Layer/ParagraphObjectAttribute.hpp"
#include "ShapeAttribute.hpp"
#include "LayerAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "MaskAttribute.hpp"
#include "TransformAttribute.hpp"
namespace VGG::layer
{
#define ATTR_MEMBER_GETTER(name, type, container)                                                  \
  type* name() const                                                                               \
  {                                                                                                \
    return container;                                                                              \
  }

class Accessor
{
public:
  Accessor(const Accessor&) = default;
  Accessor& operator=(const Accessor& a) = delete;
  Accessor(Accessor&&) = default;
  Accessor& operator=(Accessor&&) = delete;
  ~Accessor() = default;

  void setTransform(const Transform& transform)
  {
    ASSERT(m_transformAttr != nullptr);
    m_transformAttr->setTransform(transform);
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setAlphaMask(T&& alphaMasks)
  {
    ASSERT(m_alphaMaskAttr != nullptr);
    m_alphaMaskAttr->setAlphaMasks(std::forward<T>(alphaMasks));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setShapeMask(T&& maskID)
  {
    ASSERT(m_shapeMaskAttr != nullptr);
    m_shapeMaskAttr->setMaskID(std::forward<T>(maskID));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setDropShadow(T&& shadow)
  {
    m_dropShadowAttr->setDropShadowStyle(std::forward<T>(shadow));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setInnerShadow(T&& shadow)
  {
    ASSERT(m_innerShadowAttr != nullptr);
    m_innerShadowAttr->setInnerShadowStyle(std::forward<T>(shadow));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setFill(T&& fill)
  {
    ASSERT(m_styleObjectAttr != nullptr);
    m_styleObjectAttr->setFillStyle(std::forward<T>(fill));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setBorder(T&& border)
  {
    ASSERT(m_styleObjectAttr != nullptr);
    m_styleObjectAttr->setBorderStyle(std::forward<T>(border));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setLayerBlur(T&& blurs)
  {
    ASSERT(m_layerFXAttr != nullptr);
    m_layerFXAttr->setLayerBlur(std::forward<T>(blurs));
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename =
      typename std::enable_if_t<std::is_same_v<DecayT, std::vector<typename DecayT::value_type>>>>
  void setBackgroundBlur(T&& blurs)
  {
    ASSERT(m_backgroundBlurAttr != nullptr);
    m_backgroundBlurAttr->setBackgroundBlur(std::forward<T>(blurs));
  }

  ATTR_MEMBER_GETTER(transform, TransformAttribute, m_transformAttr);
  ATTR_MEMBER_GETTER(alphaMask, AlphaMaskAttribute, m_alphaMaskAttr);
  ATTR_MEMBER_GETTER(shapeMask, ShapeMaskAttribute, m_shapeMaskAttr);
  ATTR_MEMBER_GETTER(dropShadow, DropShadowAttribute, m_dropShadowAttr);
  ATTR_MEMBER_GETTER(innerShadow, InnerShadowAttribute, m_innerShadowAttr);
  ATTR_MEMBER_GETTER(styleObject, ObjectAttribute, m_styleObjectAttr); // typo: Should be object
  ATTR_MEMBER_GETTER(layerFX, LayerFXAttribute, m_layerFXAttr);
  ATTR_MEMBER_GETTER(backgroundBlur, BackgroundBlurAttribute, m_backgroundBlurAttr);

protected:
  friend class RenderNodeFactory;
  TransformAttribute* const      m_transformAttr;
  AlphaMaskAttribute* const      m_alphaMaskAttr;
  ShapeMaskAttribute* const      m_shapeMaskAttr;
  DropShadowAttribute* const     m_dropShadowAttr;
  InnerShadowAttribute* const    m_innerShadowAttr;
  ObjectAttribute* const         m_styleObjectAttr;
  LayerFXAttribute* const        m_layerFXAttr;
  BackgroundBlurAttribute* const m_backgroundBlurAttr;

  Accessor(
    TransformAttribute*      transformAttr,
    AlphaMaskAttribute*      alphaMaskAttr,
    ShapeMaskAttribute*      shapemaskAttr,
    DropShadowAttribute*     dropShadowAttr,
    InnerShadowAttribute*    innerShadowAttr,
    ObjectAttribute*         objectAttr,
    LayerFXAttribute*        layerPostProcessAttr,
    BackgroundBlurAttribute* backgroundBlurAttr)
    : m_transformAttr(transformAttr)
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

class VectorObjectAttibuteAccessor : public Accessor
{
public:
  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename = typename std::enable_if_t<std::is_same_v<DecayT, VShape>>>
  void setShape(T&& shape)
  {
    ASSERT(m_shapeAttr != nullptr);
    m_shapeAttr->setShape(std::forward<T>(shape));
  }
  ATTR_MEMBER_GETTER(shape, ShapeAttribute, m_shapeAttr);

  VectorObjectAttibuteAccessor(const Accessor& acc, ShapeAttribute* shape)
    : Accessor(acc)
    , m_shapeAttr(shape)
  {
  }

  VectorObjectAttibuteAccessor(
    ShapeAttribute*          shapeAttr,
    TransformAttribute*      transformAttr,
    AlphaMaskAttribute*      alphaMaskAttr,
    ShapeMaskAttribute*      shapemaskAttr,
    DropShadowAttribute*     dropShadowAttr,
    InnerShadowAttribute*    innerShadowAttr,
    ObjectAttribute*         objectAttr,
    LayerFXAttribute*        layerPostProcessAttr,
    BackgroundBlurAttribute* backgroundBlurAttr)
    : Accessor(
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

protected:
  ShapeAttribute* const m_shapeAttr;
};

class ParagraphAttributeAccessor : public Accessor
{
public:
  ATTR_MEMBER_GETTER(paragraph, ParagraphObjectAttribute, m_paraAttr);
  ParagraphAttributeAccessor(const Accessor& acc, ParagraphObjectAttribute* paragraphObjectAttr)
    : Accessor(acc)
    , m_paraAttr(paragraphObjectAttr)
  {
  }
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

class ImageAttribtueAccessor : public Accessor
{
public:
  ATTR_MEMBER_GETTER(image, ImageRenderObjectAttribute, m_imageAttr);
  ImageAttribtueAccessor(const Accessor& acc, ImageRenderObjectAttribute* image)
    : Accessor(acc)
    , m_imageAttr(image)
  {
  }

protected:
  ImageRenderObjectAttribute* const m_imageAttr;
};

#undef ATTR_MEMBER_GETTER
} // namespace VGG::layer
