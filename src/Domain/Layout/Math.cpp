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

#include "Math.hpp"

#include "Rect.hpp"

#include "Math/Algebra.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include <array>
#include <vector>

using namespace VGG;
using namespace Layout;

namespace
{

Matrix fromMat3(const glm::mat3& t)
{
  // t[col][row]
  return { t[0][0], t[0][1], t[1][0], t[1][1], t[2][0], t[2][1] };
}

glm::mat3 toMat3(const Matrix& m)
{
  return { m.a, m.b, 0, m.c, m.d, 0, m.tx, m.ty, 1 };
}

} // namespace

Matrix Matrix::makeInverse() const
{
  const auto t = glm::inverse(toMat3(*this));
  return fromMat3(t);
}

Matrix Matrix::rotated(Scalar radian) const
{
  auto glmMatrix = toMat3(*this);
  glmMatrix = glm::rotate(glmMatrix, static_cast<float>(radian));
  return fromMat3(glmMatrix);
}

Matrix Matrix::scaled(Scalar x, Scalar y) const
{
  auto glmMatrix = toMat3(*this);
  glmMatrix = glm::scale(glmMatrix, { static_cast<float>(x), static_cast<float>(y) });
  return fromMat3(glmMatrix);
}

Scalar Matrix::decomposeRotateRadian() const
{
  glm::vec2 scale;
  float     radian;
  glm::quat quat;
  glm::vec2 skew;
  glm::vec2 trans;
  glm::vec3 persp;

  decompose(toMat3(*this), scale, radian, quat, skew, trans, persp);

  return radian;
}

Matrix Matrix::make(Scalar tx, Scalar ty, Scalar radian)
{
  auto t = glm::mat3{ 1 };
  t = glm::translate(t, { tx, ty });
  t = glm::rotate(t, static_cast<float>(radian));
  return fromMat3(t);
}

Matrix Matrix::makeRotate(Scalar radian)
{
  const auto t = glm::rotate(glm::mat3{ 1 }, static_cast<float>(radian));
  return fromMat3(t);
}

Matrix Matrix::getAffineTransform(
  const std::array<Point, 3>& oldPoints,
  const std::array<Point, 3>& newPoints)
{
  const glm::mat3   oldGlmPoints{ oldPoints[0].x, oldPoints[0].y, 1,
                                oldPoints[1].x, oldPoints[1].y, 1,
                                oldPoints[2].x, oldPoints[2].y, 1 };
  const glm::mat3x2 newGlmPoints{ newPoints[0].x, newPoints[0].y, newPoints[1].x,
                                  newPoints[1].y, newPoints[2].x, newPoints[2].y };

  const auto mat3 = newGlmPoints * glm::inverse(oldGlmPoints);
  return fromMat3(mat3);
}
