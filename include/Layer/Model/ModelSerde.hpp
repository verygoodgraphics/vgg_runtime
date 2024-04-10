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

#include "Layer/Core/VType.hpp"
#include "Layer/Core/VBounds.hpp"
#include "Domain/Model/DesignModel.hpp"
#include <concepts>
#include <type_traits>
namespace VGG::layer::serde
{

template<typename R, typename T>
struct ModelSerde
{
  // static R serde_from(const T& t); // NOLINT
};

template<class, template<class...> class>
inline constexpr bool IsSpecialization = false; // NOLINT
template<template<class...> class T, class... Args>
inline constexpr bool IsSpecialization<T<Args...>, T> = true; // NOLINT

template<class T>
concept IsVec = IsSpecialization<T, std::vector>;

template<typename F, typename T, typename S>
concept SerderFromConcept = requires(F f, T t) {
  {
    S::serde_from(f)
  } -> std::convertible_to<T>;
};

template<typename F, typename T>
concept TrivialConvertible = requires(F f, T t) {
  requires !IsVec<std::remove_cvref_t<F>>;
  requires !IsVec<std::remove_cvref_t<T>>;
  {
    T(f)
  } -> std::convertible_to<T>;
};

template<typename F, typename T>
concept HasSerdeFrom = requires(F f, T t) {
  {
    serde_from(f)
  } -> std::convertible_to<T>;
};

template<typename F, typename T, typename S>
concept ConvertibleTo =
  std::convertible_to<F, T> || SerderFromConcept<F, T, S> || requires(F f, T t) {
    {
      T(f)
    } -> std::convertible_to<T>;
  };

template<typename F, typename T, typename S>
  requires SerderFromConcept<std::remove_cvref_t<F>, std::remove_cvref_t<T>, S>
inline T model_serde_from(const F& from) // NOLINT
{
  return S::serde_from(from);
}

template<typename F, typename T, typename S>
  requires TrivialConvertible<std::remove_cvref_t<F>, std::remove_cvref_t<T>>
inline T model_serde_from(const F& from) // NOLINT
{
  return T(from);
}

template<typename F, typename T, typename S = void>
  requires IsVec<std::remove_cvref_t<F>> && IsVec<std::remove_cvref_t<T>>
inline T model_serde_from(const F& from) // NOLINT
{
  T a;
  for (const auto& e : from)
    a.push_back(model_serde_from<
                typename F::value_type,
                typename T::value_type,
                ModelSerde<typename F::value_type, typename T::value_type>>(e));
  return a;
}
} // namespace VGG::layer::serde
