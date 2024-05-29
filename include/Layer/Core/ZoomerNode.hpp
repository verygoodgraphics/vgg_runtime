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

#include "Layer/Core/VNode.hpp"
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/VUtils.hpp"
namespace VGG::layer
{

class ZoomerNode final : public TransformNode
{
public:
  static constexpr int   ZOOM_LEVEL_COUNT = 13;
  static constexpr float ZOOM_LEVEL[ZOOM_LEVEL_COUNT] = { 1 / 4.f, 1 / 3.f, 1 / 2.f, 2 / 3.f,
                                                          3 / 4.f, 1.f,     4 / 3.f, 3 / 2.f,
                                                          2.f,     3.f,     4.f,     5.f };

  enum EScaleLevel
  {
    SL_1_4 = 0,
    SL_1_3,
    SL_1_2,
    SL_2_3,
    SL_3_4,
    SL_1_1,
    SL_4_3,
    SL_3_2,
    SL_2_1,
    SL_3_1,
    SL_4_1,
    SL_5_1,
  };

  struct OtherLevel
  {
    bool operator==(const OtherLevel& other) const
    {
      return true;
    }
  };

  using ScaleLevel = std::variant<EScaleLevel, OtherLevel>;
  using Scale = std::pair<ScaleLevel, float>;

  ZoomerNode(VRefCnt* cnt)
    : TransformNode(cnt)
  {
  }

  VGG_CLASS_MAKE(ZoomerNode);

  // setters

  void setScale(EScaleLevel level, glm::vec2 anchor = { 0, 0 })
  {
    if (level >= EScaleLevel::SL_1_4 && level <= EScaleLevel::SL_5_1)
    {
      updateScale({ level, ZOOM_LEVEL[level] }, anchor);
    }
  }

  void setScale(float zoom, glm::vec2 anchor = { 0, 0 })
  {
    updateScale({ OtherLevel{}, zoom }, anchor);
  }

  void setTranslate(float dx, float dy)
  {
    setOffset(glm::vec2(dx, dy) + m_offset);
  }

  void setOffset(glm::vec2 offset)
  {
    if (m_offset == offset)
      return;
    m_offset = offset;
    m_offsetInvalid = true;
    this->invalidate();
  }

  // setters end

  float scale() const
  {
    return m_scale.second;
  }

  ScaleLevel scaleLevel() const
  {
    return m_scale.first;
  }

  const glm::vec2& getOffset() const
  {
    return m_offset;
  }

  glm::mat3 matrix() const
  {
    auto tx = m_offset.x;
    auto ty = m_offset.y;
    auto s = m_scale.second;
    return glm::mat3{ s, 0, 0, 0, s, 0, tx, ty, 1 };
  }

  glm::mat3 getMatrix() const override
  {
    return matrix();
  }

  glm::mat3 invMatrix() const
  {
    auto tx = -m_offset.x;
    auto ty = -m_offset.y;
    auto s = 1.0 / m_scale.second;
    return glm::mat3{ s, 0, 0, 0, s, 0, tx * s, ty * s, 1 };
  }

  std::optional<EScaleLevel> discreteScaleLevel() const
  {
    std::optional<EScaleLevel> ret(std::nullopt);
    std::visit(
      layer::Overloaded{ [&](EScaleLevel level) { ret = level; }, [](OtherLevel other) {} },
      m_scale.first);
    return ret;
  }

  bool hasInvalScale() const
  {
    return m_scaleInvalid;
  }

  bool hasOffsetInval() const
  {
    return m_offsetInvalid;
  }

  Bounds onRevalidate(Invalidator* inv, const glm::mat3& mat) override
  {
    m_scaleInvalid = false;
    m_offsetInvalid = false;
    return Bounds();
  }

private:
  bool updateScale(Scale scale, glm::vec2 anchor)
  {
    if (scale == m_scale && anchor == m_anchor)
      return false;

    m_anchor = anchor;
    float dz = (scale.second - m_scale.second) / m_scale.second;
    if (scale.second > 0.01 && scale.second < 100)
    {
      m_offset.x -= (anchor.x - m_offset.x) * dz;
      m_offset.y -= (anchor.y - m_offset.y) * dz;
      m_scale = scale;
    }
    else
    {
      return false;
    }
    m_scaleInvalid = true;
    m_offsetInvalid = true;
    this->invalidate();
    return true;
  }
  glm::vec2 m_offset{ 0.f, 0.f };
  Scale     m_scale{ SL_1_1, ZOOM_LEVEL[SL_1_1] };

  glm::vec2 m_anchor{ 0.f, 0.f };
  glm::mat3 m_mat33;
  glm::mat3 m_inv33;

  bool m_scaleInvalid{ false };
  bool m_offsetInvalid{ false };
};

}; // namespace VGG::layer
