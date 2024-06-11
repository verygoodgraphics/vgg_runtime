#include "RasterManager.hpp"
#include "TileIterator.hpp"
#include "VSkia.hpp"

#include "gpu/ganesh/SkSurfaceGanesh.h"
#include "gpu/GpuTypes.h"
#include "core/SkSurface.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>

namespace
{

sk_sp<SkSurface> rasterSurface(GrRecordingContext* context, int w, int h)
{
  ASSERT(w > 0 && h > 0);
  auto info = SkImageInfo::MakeN32Premul(w, h);
  auto surf = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
  if (!surf)
  {
    return nullptr;
  }
  return surf;
}

} // namespace
namespace VGG::layer
{

void RasterManager::query(const std::vector<int>& query, std::vector<RasterResult>& result)
{
  for (auto& index : query)
  {
    auto fut = m_cache.find(index);
    if (fut)
    {
      fut->wait();
      result.push_back(fut->get());
    }
  }
}

void RasterManager::raster(std::vector<RasterTask> tasks)
{
  for (auto& task : tasks)
  {
    m_cache.insertOrUpdate(task.index, m_executor->addRasterTask(task));
  }
}

SimpleRasterExecutor::Future SimpleRasterExecutor::addRasterTask(
  RasterManager::RasterTask rasterTask)
{
  auto task = std::make_shared<std::packaged_task<RasterManager::RasterResult()>>(
    [&]() -> RasterManager::RasterResult
    {
      sk_sp<SkSurface> surf = std::move(rasterTask.surf);
      if (!rasterTask.surf)
      {
        surf = rasterSurface(context(), rasterTask.mgr->width(), rasterTask.mgr->height());
      }
      ASSERT(surf);
      auto canvas = surf->getCanvas();
      canvas->drawPicture(rasterTask.picture.get());
      return RasterManager::RasterResult(rasterTask.mgr, std::move(surf), rasterTask.index);
    });
  add([task]() { (*task)(); });
  return task->get_future();
}
} // namespace VGG::layer
