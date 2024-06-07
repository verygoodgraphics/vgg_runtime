#include "RasterCache.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
namespace VGG::layer
{

void TileProxy::blit(sk_sp<SkSurface> surf, int rx, int ry)
{
  m_target->blit(surf, rx, ry);
}

} // namespace VGG::layer
