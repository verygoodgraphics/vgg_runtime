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

#include "Layer/Core/Attrs.hpp"
#include "Layer/Core/VType.hpp"
#include "Layer/Core/VShape.hpp"
#include "Layer/Core/PaintNode.hpp"
#define UNPACK(...) __VA_ARGS__
#define C_DECL(name, type)                                                                         \
  {                                                                                                \
    a.get##name()                                                                                  \
  } -> std::convertible_to<UNPACK type>

using Float4 = std::array<float, 4>;
using Float2 = std::array<float, 2>;

namespace VGG::layer
{

enum class EModelObjectType
{
  GROUP,
  FRAME,
  PATH,
  IMAGE,
  TEXT,
  CONTOUR,
  MASTER,
  INSTANCE,
  UNKNOWN
};

template<class R, class F, class... Args>
concept CallableObject = std::is_invocable_r<R, F, Args...>::value;

template<typename T>
concept AbstractObject = requires(T a) {
  typename T::BaseType;
  typename T::Self;
  C_DECL(ObjectType, (EModelObjectType));
  C_DECL(ObjectTypeString, (std::string));
  C_DECL(Name, (std::string));
  C_DECL(Id, (std::string));
  C_DECL(Bounds, (Bounds));
  C_DECL(Matrix, (glm::mat3));
  C_DECL(Visible, (bool));
  C_DECL(Overflow, (EOverflow));
  C_DECL(Style, (Style));
  C_DECL(ContextSetting, (ContextSetting));
  C_DECL(CornerSmoothing, (float));
  C_DECL(MaskType, (EMaskType));
  C_DECL(MaskShowType, (EMaskShowType));
  C_DECL(ShapeMask, (std::vector<std::string>));
  C_DECL(AlphaMask, (std::vector<AlphaMask>));
  C_DECL(ChildObjects, (std::vector<typename T::BaseType>));
};

template<typename ObjectType>
  requires AbstractObject<ObjectType>
struct SubShape
{
  SubShape(EBoolOp bop, std::variant<ObjectType, ShapeData> geo)
    : booleanOperation(bop)
    , geometry(geo)
  {
  }
  EBoolOp                             booleanOperation;
  std::variant<ObjectType, ShapeData> geometry;
};

template<typename T>
concept GroupObject = AbstractObject<T>;

template<typename T>
concept FrameObject =
  AbstractObject<T> && requires(T a) { C_DECL(Radius, (std::array<float, 4>)); };

template<typename T>
concept MasterObject =
  AbstractObject<T> && requires(T a) { C_DECL(Radius, (std::array<float, 4>)); };

template<typename T>
concept InstanceObject = AbstractObject<T>;

template<typename T>
concept PathObject = AbstractObject<T> && requires(T a) {
  C_DECL(WindingType, (EWindingType));
  C_DECL(Shapes, (std::vector<SubShape<typename T::BaseType>>));
};

template<typename T>
concept ImageObject = AbstractObject<T> && requires(T a) {
  C_DECL(ImageBounds, (Bounds));
  C_DECL(ImageGUID, (std::string));
  C_DECL(ImageFilter, (ImageFilter));
};

template<typename T, typename BaseType = T>
concept TextObject = AbstractObject<T> && requires(T a) {
  C_DECL(Text, (std::string));
  C_DECL(TextBounds, (Bounds));
  C_DECL(VerticalAlignment, (ETextVerticalAlignment));
  C_DECL(LayoutMode, (ETextLayoutMode));
  C_DECL(Anchor, (std::optional<Float2>));
  C_DECL(TextLineType, (std::vector<TextLineAttr>));
  C_DECL(HorizontalAlignment, (std::vector<ETextHorizontalAlignment>));
  C_DECL(DefaultFontAttr, (TextStyleAttr));
  C_DECL(FontAttr, (std::vector<TextStyleAttr>));
};

template<typename T, typename U>
concept CastObject = AbstractObject<U> && requires(T a) {
  {
    T::asGroup(std::declval<U>())
  } -> GroupObject;
  {
    T::asFrame(std::declval<U>())
  } -> FrameObject;
  {
    T::asMaster(std::declval<U>())
  } -> MasterObject;
  {
    T::asInstance(std::declval<U>())
  } -> InstanceObject;
  {
    T::asImage(std::declval<U>())
  } -> ImageObject;
  {
    T::asText(std::declval<U>())
  } -> TextObject;
  {
    T::asPath(std::declval<U>())
  } -> PathObject;
};

template<typename T>
concept ModelConcept = requires(T a) {
  typename T::Model;
  typename T::CastObject;
  requires AbstractObject<typename T::Model> || GroupObject<typename T::Model> ||
             FrameObject<typename T::Model> || MasterObject<typename T::Model> ||
             InstanceObject<typename T::Model> || ImageObject<typename T::Model> ||
             TextObject<typename T::Model> || PathObject<typename T::Model>;
  requires CastObject<typename T::Model, T>;
};

#undef UNPACK
#undef C_DECL

} // namespace VGG::layer
