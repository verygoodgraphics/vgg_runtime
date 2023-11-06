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

#include "Domain/Layout/Rect.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>

namespace VGG
{

namespace Layout
{

struct Matrix
{
  Scalar a{ 1 };
  Scalar b{ 0 };
  Scalar c{ 0 };
  Scalar d{ 1 };
  Scalar tx{ 0 };
  Scalar ty{ 0 };

  bool operator==(const Matrix& rhs) const noexcept;
  Matrix makeInverse() const;
  Scalar decomposeRotateRadian() const;

  static Matrix make(Scalar tx, Scalar ty, Scalar radian);
  static Matrix makeRotate(Scalar radian);
  static Matrix make(glm::mat3 t);

  static Matrix getAffineTransform(const std::array<Point, 3>& oldPoints,
                                   const std::array<Point, 3>& newPoints);

private:
  glm::mat3 makeMat3() const;
};

} // namespace Layout

} // namespace VGG