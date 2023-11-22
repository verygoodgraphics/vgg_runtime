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
#include "Layer/AttrSerde.hpp"
#include "Layer/Core/VType.hpp"
#include "VSkia.hpp"
#include "Layer/Config.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <include/core/SkMatrix.h>
#include <include/core/SkCanvas.h>

// NOLINTBEGIN
inline void PrintSkMatrix(const SkMatrix& m)
{
  for (int i = 0; i < 9; i++)
  {
    std::cout << m[i] << " ";
  }
  std::cout << std::endl;
}

struct DisplayItem
{
  SkMatrix   matrix;
  PaintNode* item{ nullptr };
  int        zorder = 0;

  DisplayItem(SkMatrix m, PaintNode* item, int zorder)
    : matrix(std::move(m))
    , item(item)
    , zorder(zorder)
  {
  }
};

inline std::ostream& operator<<(std::ostream& os, const glm::mat3 m)
{
  os << "[" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << std::endl;
  os << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << std::endl;
  os << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << "]\n";
  return os;
}

namespace VGG::layer
{
// NOLINTEND
class SkiaRenderer
{
  SkCanvas*                                   m_canvas{ nullptr };
  bool                                        m_enableDrawDebugBound{ false };
  std::unordered_map<std::string, PaintNode*> m_maskObjects;
  Scene*                                      m_scene{ nullptr };
  std::vector<glm::mat3>                      m_transforms;
  std::vector<DisplayItem>                    m_displayList;
  int                                         m_zOrder = -1;

  SkMatrix m_currentMatrix;

  void updateMaskObject(PaintNode* p, std::unordered_map<std::string, PaintNode*>& objects);

public:
  void drawDebugBound(layer::PaintNode* p, int zorder);
  void updateMaskObject(layer::PaintNode* p);
  const std::unordered_map<std::string, layer::PaintNode*>& maskObjects() const
  {
    return m_maskObjects;
  }

  void draw(SkCanvas* canvas, layer::PaintNode* root);

  void pushMatrix(glm::mat3 matrix)
  {
    m_transforms.push_back(matrix);
  }

  void pushItem(layer::PaintNode* item, int zorder)
  {
    // OPTIMIZATION
    auto m = glm::identity<glm::mat3>();
    for (const auto& ma : m_transforms)
    {
      m = m * ma;
    }
    m_displayList.emplace_back(toSkMatrix(m), item, zorder);
  }

  void popMatrix()
  {
    m_transforms.pop_back();
  }

  void clearCache()
  {
    m_displayList.clear();
    m_transforms.clear();
  }

  void enableDrawDebugBound(bool enabled)
  {
    m_enableDrawDebugBound = enabled;
  }
  bool isEnableDrawDebugBound()
  {
    return m_enableDrawDebugBound;
  }

  SkCanvas* canvas()
  {
    return m_canvas;
  }

  const SkMatrix& currentMatrix()
  {
    return m_currentMatrix;
  }

  void commit(SkCanvas* canvas);
};

} // namespace VGG::layer
