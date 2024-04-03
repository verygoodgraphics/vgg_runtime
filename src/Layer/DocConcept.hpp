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
#define UNPACK(...) __VA_ARGS__
#define C_DECL(name, type)                                                                         \
  {                                                                                                \
    a.get##name()                                                                                  \
  } -> std::convertible_to<UNPACK type>

using Float4 = std::array<float, 4>;
using Float2 = std::array<float, 2>;

namespace VGG::layer
{

template<typename T>
concept AttrType =
  requires(T a) { std::same_as<T, ContextSetting> || std::same_as<T, PatternTile>; };

template<typename T>
concept BoundsObject = requires(T a) {
  C_DECL(X, (float));
  C_DECL(Y, (float));
  C_DECL(W, (float));
  C_DECL(H, (float));
};

template<typename T>
concept AbstractObject = requires(T a) {
  C_DECL(ObjectType, (EObjectType));
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
  {
    a.getChildObjects()
  };
};

template<typename T>
concept FrameObject =
  AbstractObject<T> && requires(T a) { C_DECL(Radius, (std::array<float, 4>)); };

template<typename T>
concept GroupObject = AbstractObject<T>;

template<typename T>
concept ImageObject = AbstractObject<T> && requires(T a) {
  C_DECL(ImageBounds, (Bounds));
  C_DECL(ImageGUID, (std::string));
  C_DECL(ImageFilter, (ImageFilter));
};

template<typename T>
concept TextObject = AbstractObject<T> && requires(T a) {
  C_DECL(Text, (std::string));
  C_DECL(TextBounds, (Bounds));
  C_DECL(VerticalAlignment, (ETextVerticalAlignment));
  C_DECL(LayoutMode, (ETextLayoutMode));
  C_DECL(Anchor, (std::array<float, 2>));
  C_DECL(DefaultFontAttr, (glm::vec2));
};

#undef UNPACK
#undef C_DECL

} // namespace VGG::layer
