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
#include "Layer/Core/VType.hpp"

#include "Layer/Core/VBounds.hpp"
#include "Layer/Core/PaintNode.hpp"

#include <vector>
#include <string>
#include <optional>

using Float4 = std::array<float, 4>;
using Float2 = std::array<float, 2>;

namespace VGG::layer
{

enum class EModelObjectType
{
  OBJECT,
  GROUP,
  FRAME,
  PATH,
  IMAGE,
  TEXT,
  MASTER,
  INSTANCE,
  UNKNOWN
};

enum class EModelShapeType
{
  CONTOUR,
  RECTANGLE,
  ELLIPSE,
  POLYGON,
  STAR,
  VECTORNETWORK,
  UNKNOWN
};

#define DEF_MODEL_TYPE(enum) static constexpr EModelObjectType ObjectType = enum;

#define DECL_MODEL(klass, base, type)                                                              \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(type)

#define DECL_MODEL_OBJECT(klass, base)                                                             \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::OBJECT)

#define DECL_MODEL_GROUP(klass, base)                                                              \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::GROUP)

#define DECL_MODEL_FRAME(klass, base)                                                              \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::FRAME)

#define DECL_MODEL_PATH(klass, base)                                                               \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::PATH)

#define DECL_MODEL_IMAGE(klass, base)                                                              \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::IMAGE)

#define DECL_MODEL_TEXT(klass, base)                                                               \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::TEXT)

#define DECL_MODEL_MASTER(klass, base)                                                             \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::MASTER)

#define DECL_MODEL_INSTANCE(klass, base)                                                           \
  using BaseType = base;                                                                           \
  using Self = klass;                                                                              \
  DEF_MODEL_TYPE(EModelObjectType::INSTANCE)

#define D_OBJECT_MODEL DEF_MODEL_TYPE(OBJECT)

template<class R, class F, class... Args>
concept CallableObject = std::is_invocable_r<R, F, Args...>::value;

#define UNPACK(...) __VA_ARGS__
#define REQ_DECL(name, type)                                                                       \
  {                                                                                                \
    a.get##name()                                                                                  \
  } -> std::convertible_to<UNPACK type>

template<typename T>
concept AbstractObject = requires(T a) {
  typename T::BaseType;
  typename T::Self;
  {
    T::ObjectType
  } -> std::convertible_to<EModelObjectType>;
  REQ_DECL(ObjectType, (EModelObjectType));
  // REQ_DECL(ObjectTypeString, (std::string));
  REQ_DECL(UniqueId, (int));
  REQ_DECL(Name, (std::string));
  REQ_DECL(Id, (std::string));
  REQ_DECL(Bounds, (Bounds));
  REQ_DECL(Matrix, (glm::mat3));
  REQ_DECL(Visible, (bool));
  REQ_DECL(Overflow, (EOverflow));
  REQ_DECL(Style, (Style));
  REQ_DECL(ContextSetting, (ContextSetting));
  REQ_DECL(CornerSmoothing, (float));
  REQ_DECL(MaskType, (EMaskType));
  REQ_DECL(MaskShowType, (std::optional<EMaskShowType>));
  REQ_DECL(ShapeMask, (std::vector<std::string>));
  REQ_DECL(AlphaMask, (std::vector<AlphaMask>));
  REQ_DECL(ChildObjects, (std::vector<typename T::BaseType>));
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
concept GroupObject =
  AbstractObject<T> && requires(T a) { requires T::ObjectType == EModelObjectType::GROUP; };

template<typename T>
concept FrameObject = AbstractObject<T> && requires(T a) {
  requires T::ObjectType == EModelObjectType::FRAME;
  REQ_DECL(Radius, (std::array<float, 4>));
};

template<typename T>
concept MasterObject = AbstractObject<T> && requires(T a) {
  requires T::ObjectType == EModelObjectType::MASTER;
  REQ_DECL(Radius, (std::array<float, 4>));
};

template<typename T>
concept InstanceObject =
  AbstractObject<T> && requires(T a) { requires T::ObjectType == EModelObjectType::INSTANCE; };

template<typename T>
concept PathObject = AbstractObject<T> && requires(T a) {
  requires T::ObjectType == EModelObjectType::PATH;
  REQ_DECL(WindingType, (EWindingType));
  REQ_DECL(Shapes, (std::vector<SubShape<typename T::BaseType>>));
};

template<typename T>
concept ImageObject = AbstractObject<T> && requires(T a) {
  requires T::ObjectType == EModelObjectType::IMAGE;
  REQ_DECL(ImageBounds, (Bounds));
  REQ_DECL(ImageGUID, (std::string));
  REQ_DECL(ImageFilter, (ImageFilter));
};

template<typename T, typename BaseType = T>
concept TextObject = AbstractObject<T> && requires(T a) {
  requires T::ObjectType == EModelObjectType::TEXT;
  REQ_DECL(Text, (std::string));
  REQ_DECL(TextBounds, (Bounds));
  REQ_DECL(VerticalAlignment, (ETextVerticalAlignment));
  REQ_DECL(LayoutMode, (ETextLayoutMode));
  REQ_DECL(Anchor, (std::optional<Float2>));
  REQ_DECL(TextLineType, (std::vector<TextLineAttr>));
  REQ_DECL(HorizontalAlignment, (std::vector<ETextHorizontalAlignment>));
  REQ_DECL(DefaultFontAttr, (TextStyleAttr));
  REQ_DECL(FontAttr, (std::vector<TextStyleAttr>));
  REQ_DECL(OverrideFontAttr, (std::vector<TextStyleAttr>));
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

template<typename M, typename C>
struct ModelPolicy
{
  using Model = M;
  using CastObject = C;
};

#undef UNPACK
#undef REQ_DECL

} // namespace VGG::layer
