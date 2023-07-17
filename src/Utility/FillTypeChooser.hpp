/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __FILL_TYPE_CHOOSER_HPP__
#define __FILL_TYPE_CHOOSER_HPP__

#include <functional>

#include <Components/Styles.hpp>

#define CHOOSER(T, X)                                                                              \
  if (ImGui::Selectable(#X, FillTypeChooser<T>::is##X(style)))                                     \
  {                                                                                                \
    FillTypeChooser<T>::set##X(style);                                                             \
  }

namespace VGG
{

template<typename StyleType>
struct FillTypeChooser
{
  using FT = typename StyleType::FillType;
  using GT = Gradient::GradientType;

  inline static const char* fillTypeLabels[] = { "Flat",           "LinearGradient",
                                                 "RadialGradient", "AngularGradient",
                                                 "Image",          "Noise" };

  inline static const char* getLabel(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    if constexpr (std::is_same_v<StyleType, FillStyle>)
    {
      std::vector<std::function<bool(const StyleType&)>> funcs{ isFlat,           isLinearGradient,
                                                                isRadialGradient, isAngularGradient,
                                                                isImage,          isNoise };
      for (size_t i = 0; i < funcs.size(); i++)
      {
        if (funcs[i](style))
        {
          return fillTypeLabels[i];
        }
      }
    }
    else if constexpr (std::is_same_v<StyleType, BorderStyle>)
    {
      std::vector<std::function<bool(const StyleType&)>> funcs{ isFlat,
                                                                isLinearGradient,
                                                                isRadialGradient,
                                                                isAngularGradient };
      for (size_t i = 0; i < funcs.size(); i++)
      {
        if (funcs[i](style))
        {
          return fillTypeLabels[i];
        }
      }
    }
    return nullptr;
  }

  inline static bool isFlat(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    return style.fillType == FT::FLAT;
  }

  inline static void setFlat(StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    style.fillType = FT::FLAT;
  }

  inline static bool isGradient(const StyleType& style)
  {
    return isLinearGradient(style) || isRadialGradient(style) || isAngularGradient(style);
  }

  inline static bool isLinearGradient(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    return (style.fillType == FT::GRADIENT) && (style.gradient.gradientType == GT::LINEAR);
  }

  inline static void setLinearGradient(StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    style.fillType = FT::GRADIENT;
    style.gradient.gradientType = GT::LINEAR;
  }

  inline static bool isRadialGradient(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    return (style.fillType == FT::GRADIENT) && (style.gradient.gradientType == GT::RADIAL);
  }

  inline static void setRadialGradient(StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    style.fillType = FT::GRADIENT;
    style.gradient.gradientType = GT::RADIAL;
  }

  inline static bool isAngularGradient(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    return (style.fillType == FT::GRADIENT) && (style.gradient.gradientType == GT::ANGULAR);
  }

  inline static void setAngularGradient(StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle> || std::is_same_v<StyleType, BorderStyle>);
    style.fillType = FT::GRADIENT;
    style.gradient.gradientType = GT::ANGULAR;
  }

  inline static bool isImage(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle>);
    return style.fillType == FT::IMAGE;
  }

  inline static void setImage(StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle>);
    style.fillType = FT::IMAGE;
  }

  inline static bool isNoise(const StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle>);
    return style.fillType == FT::NOISE;
  }

  inline static void setNoise(StyleType& style)
  {
    static_assert(std::is_same_v<StyleType, FillStyle>);
    style.fillType = FT::NOISE;
  }
};

}; // namespace VGG

#endif // __FILL_TYPE_CHOOSER_HPP__
