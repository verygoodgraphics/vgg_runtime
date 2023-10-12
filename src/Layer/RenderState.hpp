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
#include <optional>
#include <stack>

#include "Layer/Core/PaintNode.hpp"

namespace VGG
{

using SceneCacheFlags = uint32_t;

enum ERenderStateFlagsBits : SceneCacheFlags
{
  D_MASK = 1,
  D_ALL = D_MASK
};
class RenderState
{
  float m_alpha{ 1.0 };
  SceneCacheFlags m_cacheFlags{ ERenderStateFlagsBits::D_ALL };

public:
  RenderState()
  {
  }

  bool isDirty() const
  {
    return m_cacheFlags != 0;
  }

  void clear(ERenderStateFlagsBits bit)
  {
    m_cacheFlags &= ~bit;
  }

  void set(ERenderStateFlagsBits bit)
  {
    m_cacheFlags |= bit;
  }

  bool test(ERenderStateFlagsBits bit) const
  {
    return m_cacheFlags & bit;
  }

  bool testAll(ERenderStateFlagsBits bit) const
  {
    return (m_cacheFlags & bit) == bit;
  }

  void preprocessMask(PaintNode* node)
  {
    if (test(D_MASK))
    {
      // generate each mask for masked node
      clear(D_MASK);
    }
  }
};

}; // namespace VGG
