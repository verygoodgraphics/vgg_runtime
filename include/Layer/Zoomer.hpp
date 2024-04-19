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
#include "Layer/Core/VUtils.hpp"
#include "Math/Math.hpp"
#include "Layer/Core/VBounds.hpp"
#include <optional>
#include <variant>

using namespace VGG;

class SkCanvas;

namespace VGG
{
class Scene;
}

class [[deprecated("Using ZoomerNode instead")]] Zoomer
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
  };

  using ScaleLevel = std::variant<EScaleLevel, OtherLevel>;
  using Scale = std::pair<ScaleLevel, float>;

  void setOwnerScene(Scene* owner);

  glm::mat3 matrix() const
  {
    auto tx = m_offset.x;
    auto ty = m_offset.y;
    auto s = m_scale.second;
    return glm::mat3{ s, 0, 0, 0, s, 0, tx, ty, 1 };
  }

  glm::mat3 invMatrix() const
  {
    auto tx = -m_offset.x;
    auto ty = -m_offset.y;
    auto s = 1.0 / m_scale.second;
    return glm::mat3{ s, 0, 0, 0, s, 0, tx * s, ty * s, 1 };
  }

  // translation
  glm::vec2 translate() const
  {
    return m_offset;
  }
  void setOffset(glm::vec2 offset);
  void setTranslate(float dx, float dy);

  // scale
  float scale() const
  {
    return m_scale.second;
  }

  ScaleLevel scaleLevel() const
  {
    return m_scale.first;
  }

  std::optional<EScaleLevel> discreteScaleLevel() const
  {
    std::optional<EScaleLevel> ret(std::nullopt);
    std::visit(
      layer::Overloaded{ [&](EScaleLevel level) { ret = level; }, [](OtherLevel other) {} },
      m_scale.first);
    return ret;
  }
  void setScale(EScaleLevel level, glm::vec2 anchor = { 0, 0 });
  void setScale(float zoom, glm::vec2 anchor = { 0, 0 })
  {
    updateScale({ OtherLevel{}, zoom }, anchor);
  }

  void mapCanvasPosToLogicalPosition(const float canvasXY[2], float logicXY[2]) const;

private:
  bool      updateScale(Scale scale, glm::vec2 anchor);
  Scene*    m_owner{ nullptr };
  glm::vec2 m_offset{ 0.f, 0.f };
  Scale     m_scale{ SL_1_1, ZOOM_LEVEL[SL_1_1] };
};
