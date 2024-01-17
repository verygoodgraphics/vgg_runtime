#include "LayerCache.h"

namespace VGG::layer
{

BlenderCache* GlobalBlenderCache()
{
  static BlenderCache s_blenderCache(40);
  return &s_blenderCache;
}

EffectCache* GlobalEffectCache()
{
  static EffectCache s_blenderCache(40);
  return &s_blenderCache;
}

} // namespace VGG::layer
