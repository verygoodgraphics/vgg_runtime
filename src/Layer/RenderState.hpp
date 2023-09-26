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
