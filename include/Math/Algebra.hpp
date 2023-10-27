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

#include <glm/gtx/matrix_decompose.hpp>

namespace VGG
{
inline bool decompose(const glm::mat3& mat,
                      glm::vec2& scale,
                      float& angle,
                      glm::quat& quat,
                      glm::vec2& skew,
                      glm::vec2& trans,
                      glm::vec3& persp)
{
  glm::mat4 mat4{ glm::vec4{ mat[0], 0 },
                  glm::vec4{ mat[1], 0 },
                  glm::vec4{ 0, 0, 1, 0 },
                  glm::vec4{ mat[2][0], mat[2][1], 0, 1 } };
  glm::vec3 scale3;
  glm::vec3 trans3;
  glm::vec3 skew3;
  glm::vec4 persp4;
  const auto ok = glm::decompose(mat4, scale3, quat, trans3, skew3, persp4);
  scale = scale3;
  angle = glm::roll(quat);
  skew = skew3;
  trans = trans3;
  persp = persp4;
  return ok;
}
} // namespace VGG
