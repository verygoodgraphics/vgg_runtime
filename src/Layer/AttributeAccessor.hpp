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
#include "ShapeAttribute.hpp"
#include "LayerAttribute.hpp"
#include "ObjectAttribute.hpp"
#include "MaskAttribute.hpp"
#include "TransformAttribute.hpp"
namespace VGG::layer
{

class AttributeAccessor
{
public:
  AttributeAccessor(const AttributeAccessor&) = delete;
  AttributeAccessor& operator=(const AttributeAccessor&) = delete;
  AttributeAccessor(AttributeAccessor&&) = delete;
  AttributeAccessor& operator=(AttributeAccessor&&) = delete;
  ~AttributeAccessor() = default;

  void setTransform(const Transform& transform)
  {
    ASSERT(m_transformAttr != nullptr);
    m_transformAttr->setTransform(transform);
  }

  template<
    typename T,
    typename DecayT = std::decay_t<T>,
    typename = typename std::enable_if_t<std::is_same_v<DecayT, VShape>>>
  void setShape(T&& shape)
  {
    ASSERT(m_shapeAttr != nullptr);
    m_shapeAttr->setShape(std::forward<T>(shape));
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

#define ATTR_MEMBER_GETTER(name, type, container)                                                  \
  type* name() const                                                                               \
  {                                                                                                \
    return container;                                                                              \
  }

  ATTR_MEMBER_GETTER(transform, TransformAttribute, m_transformAttr);
  ATTR_MEMBER_GETTER(alphaMask, AlphaMaskAttribute, m_alphaMaskAttr);
  ATTR_MEMBER_GETTER(shapeMask, ShapeMaskAttribute, m_shapeMaskAttr);
  ATTR_MEMBER_GETTER(shape, ShapeAttribute, m_shapeAttr);
  ATTR_MEMBER_GETTER(dropShadow, DropShadowAttribute, m_dropShadowAttr);
  ATTR_MEMBER_GETTER(innerShadow, InnerShadowAttribute, m_innerShadowAttr);
  ATTR_MEMBER_GETTER(styleObject, ObjectAttribute, m_styleObjectAttr);
  ATTR_MEMBER_GETTER(layerFX, LayerFXAttribute, m_layerFXAttr);
  ATTR_MEMBER_GETTER(backgroundBlur, BackgroundBlurAttribute, m_backgroundBlurAttr);

#undef ATTR_MEMBER_GETTER

private:
  friend class DefaultRenderNode;
  friend class RenderNodeFactory;
  TransformAttribute* const      m_transformAttr;
  AlphaMaskAttribute* const      m_alphaMaskAttr;
  ShapeMaskAttribute* const      m_shapeMaskAttr;
  ShapeAttribute* const          m_shapeAttr;
  DropShadowAttribute* const     m_dropShadowAttr;
  InnerShadowAttribute* const    m_innerShadowAttr;
  ObjectAttribute* const         m_styleObjectAttr;
  LayerFXAttribute* const        m_layerFXAttr;
  BackgroundBlurAttribute* const m_backgroundBlurAttr;

  AttributeAccessor(
    TransformAttribute*      transformAttr,
    ShapeAttribute*          shapeAttr,
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
    , m_shapeAttr(shapeAttr)
    , m_dropShadowAttr(dropShadowAttr)
    , m_innerShadowAttr(innerShadowAttr)
    , m_styleObjectAttr(objectAttr)
    , m_layerFXAttr(layerPostProcessAttr)
    , m_backgroundBlurAttr(backgroundBlurAttr)
  {
  }
  friend class RenderNode;
};
} // namespace VGG::layer
