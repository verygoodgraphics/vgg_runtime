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

#include <Math/Algebra.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

namespace VGG::layer
{

class Transform
{
  glm::vec2 m_offset;
  glm::vec2 m_scale;
  float m_rotate;
  glm::mat3 m_matrix;
  mutable uint8_t m_dirty{ 0 }; // 0 clean, 1 component dirty, 2 matrix dirty

  void updateDecomposeFromMatrix()
  {
    if (m_dirty == 2)
    {
      glm::quat quat;
      glm::vec2 skew;
      glm::vec3 persp;
      decompose(m_matrix, m_scale, m_rotate, quat, skew, m_offset, persp);
      m_dirty = 0;
    }
  }

  void ensureUpdate() const
  {
    const_cast<Transform*>(this)->updateDecomposeFromMatrix();
  }

  void updateMatrixFromDecompose()
  {
    if (m_dirty == 1)
    {
      m_matrix = glm::mat3{ 1.0 };
      m_matrix = glm::scale(m_matrix, m_scale);
      m_matrix = glm::translate(m_matrix, m_offset);
      m_matrix = glm::rotate(m_matrix, m_rotate);
      m_dirty = 0;
    }
  }

  void ensureUpdateMatrix() const
  {
    const_cast<Transform*>(this)->updateMatrixFromDecompose();
  }

public:
  Transform(const glm::mat3& mat)
    : m_matrix(mat)
    , m_dirty(2)
  {
  }

  Transform(const glm::vec2& offset, const glm::vec2& scale, float rotate)
    : m_offset(offset)
    , m_scale(scale)
    , m_rotate(rotate)
    , m_dirty(1)
  {
  }

  Transform(const Transform& other) = default;
  Transform& operator=(const Transform& other) = default;
  Transform(Transform&& other) noexcept = default;
  Transform& operator=(Transform& other) noexcept = default;

  void setTranslate(float sx, float sy)
  {
    m_dirty = 1;
    m_offset = { sx, sy };
  }

  void setScale(float sx, float sy)
  {
    m_dirty = 1;
    m_scale = { sx, sy };
  }

  void setMatrix(const glm::mat3& mat)
  {
    m_dirty = 2;
    m_matrix = mat;
  }

  void setRotate(float radians)
  {
    m_dirty = 1;
    m_rotate = radians;
  }

  const glm::vec2& translate() const
  {
    ensureUpdate();
    return m_offset;
  }

  float rotate() const
  {
    ensureUpdate();
    return m_rotate;
  }

  const glm::vec2& scale() const
  {
    ensureUpdate();
    return m_scale;
  }

  const glm::mat3& matrix() const
  {
    ensureUpdateMatrix();
    return m_matrix;
  }

  Transform operator*(const Transform& other)
  {
    return Transform(this->matrix() * other.matrix());
  }
};

} // namespace VGG::layer
