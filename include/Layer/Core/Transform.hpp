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
  enum EFlags
  {
    COMP_INVAL = 1 << 0,
    MAT_INVAL = 1 << 1,
    INV_INVAL = 1 << 2,
  };
  glm::vec2 m_offset{ 0, 0 };
  glm::vec2 m_scale{ 1, 1 };
  float     m_rotate{ 0 };
  glm::mat3 m_matrix{ 1 };
  glm::mat3 m_inv{ 1 };
  uint8_t   m_flags{ 0 };

  void revalidateDecompose()
  {
    if (m_flags & COMP_INVAL)
    {
      glm::quat quat;
      glm::vec2 skew;
      glm::vec3 persp;
      decompose(m_matrix, m_scale, m_rotate, quat, skew, m_offset, persp);
      m_flags &= ~COMP_INVAL;
    }
  }

  void revalidateInvsereMatrix()
  {
    if (m_flags & INV_INVAL)
    {
      m_inv = glm::inverse(m_matrix);
      m_flags &= ~INV_INVAL;
    }
  }

  void ensureDecomposeValidate() const
  {
    const_cast<Transform*>(this)->revalidateDecompose();
  }

  void revalidateMatrix()
  {
    if (m_flags == MAT_INVAL)
    {
      m_matrix = glm::mat3{ 1.0 };
      m_matrix = glm::scale(m_matrix, m_scale);
      m_matrix = glm::translate(m_matrix, m_offset);
      m_matrix = glm::rotate(m_matrix, m_rotate);
      m_flags &= MAT_INVAL;
    }
  }

  void ensureRevalidateMatrix() const
  {
    const_cast<Transform*>(this)->revalidateMatrix();
  }

  void ensureRevalidateInverseMatrix() const
  {
    ensureRevalidateMatrix();
    const_cast<Transform*>(this)->revalidateInvsereMatrix();
  }

public:
  Transform()
  {
  }
  explicit Transform(const glm::mat3& mat)
    : m_matrix(mat)
    , m_flags(COMP_INVAL | INV_INVAL)
  {
  }

  Transform(const glm::vec2& offset, const glm::vec2& scale, float rotate)
    : m_offset(offset)
    , m_scale(scale)
    , m_rotate(rotate)
    , m_flags(MAT_INVAL | INV_INVAL)
  {
  }

  Transform(const Transform& other) = default;
  Transform& operator=(const Transform& other) = default;
  Transform(Transform&& other) noexcept = default;
  Transform& operator=(Transform& other) noexcept = default;

  void setTranslate(float tx, float ty)
  {
    ensureDecomposeValidate();
    m_offset = { tx, ty };
    m_flags |= MAT_INVAL | INV_INVAL;
  }

  void setScale(float sx, float sy)
  {
    ensureDecomposeValidate();
    m_scale = { sx, sy };
    m_flags |= MAT_INVAL | INV_INVAL;
  }

  void setRotate(float radians)
  {
    ensureDecomposeValidate();
    m_rotate = radians;
    m_flags |= MAT_INVAL | INV_INVAL;
  }

  void setMatrix(const glm::mat3& mat)
  {
    m_matrix = mat;
    m_flags |= COMP_INVAL | INV_INVAL;
  }

  const glm::vec2& translate() const
  {
    ensureDecomposeValidate();
    return m_offset;
  }

  float rotate() const
  {
    ensureDecomposeValidate();
    return m_rotate;
  }

  const glm::vec2& scale() const
  {
    ensureDecomposeValidate();
    return m_scale;
  }

  const glm::mat3& matrix() const
  {
    ensureRevalidateMatrix();
    return m_matrix;
  }

  const glm::mat3& inverse() const
  {
    ensureRevalidateInverseMatrix();
    return m_inv;
  }

  Transform operator*(const Transform& other)
  {
    return Transform(this->matrix() * other.matrix());
  }

  glm::vec3 operator*(const glm::vec3& other) const
  {
    return this->matrix() * other;
  }
};

} // namespace VGG::layer
