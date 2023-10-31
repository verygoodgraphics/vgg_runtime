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

#include <glm/glm.hpp>

#include <array>
#include <vector>

using namespace VGG;
using namespace Layout;

Matrix Matrix::makeInverse() const
{
  const glm::mat3 glmMatrix{ a, b, 0, a, d, 0, tx, ty, 1 };
  const auto inversedGlmMatirx = glm::inverse(glmMatrix);

  return {};
}

Matrix Layout::getAffineTransform(const std::array<Point, 3>& oldPoints,
                                  const std::array<Point, 3>& newPoints)
{
  const glm::mat3 oldGlmPoints{ oldPoints[0].x, oldPoints[0].y, 1,
                                oldPoints[1].x, oldPoints[1].y, 1,
                                oldPoints[2].x, oldPoints[2].y, 1 };
  const glm::mat3x2 newGlmPoints{ newPoints[0].x, newPoints[0].y, newPoints[1].x,
                                  newPoints[1].y, newPoints[2].x, newPoints[2].y };

  const auto gmMatrix = newGlmPoints * glm::inverse(oldGlmPoints);
  return {
    gmMatrix[0].x, gmMatrix[0].y, gmMatrix[1].x, gmMatrix[1].y, gmMatrix[2].x, gmMatrix[2].y
  };
}
