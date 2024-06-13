#include "RasterManager.hpp"
#include "TileIterator.hpp"
#include "VSkia.hpp"

#include "gpu/ganesh/SkSurfaceGanesh.h"
#include "gpu/GpuTypes.h"
#include "core/SkSurface.h"

#include <algorithm>
#include <iostream>
#include <optional>
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
  wait();
  // TODO::
}

void RasterManager::wait()
{
  while (!m_tasks.empty())
  {
    auto& t = m_tasks.front();
    t.second.wait();
    auto res = t.second.get();
    m_cache.insertOrUpdate(t.first, std::move(res));
    m_tasks.pop();
  }
}

void RasterManager::raster(std::vector<TileTask> tasks)
{
  for (auto& task : tasks)
  {
    m_tasks.push({ task.index, m_executor->addRasterTask(task) });
  }
}

std::optional<RasterManager::RasterResult> RasterManager::query(int index)
{
  wait();
  auto res = m_cache.find(index);
  if (res)
  {
    return *res;
  }
  return std::nullopt;
}

void RasterManager::raster(int key, RasterTask task)
{
  m_tasks.push({ key, m_executor->addRasterTask(std::move(task)) });
}

SimpleRasterExecutor::Future SimpleRasterExecutor::addRasterTask(TileTask rasterTask)
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
      canvas->restore();
      return RasterManager::RasterResult(rasterTask.mgr, std::move(surf), rasterTask.index);
    });
  add([task]() { (*task)(); });
  return task->get_future();
}

SimpleRasterExecutor::Future SimpleRasterExecutor::addRasterTask(RasterTask rasterTask)
{
  auto task = std::make_shared<std::packaged_task<RasterManager::RasterResult()>>(
    [&]() -> RasterManager::RasterResult
    {
      sk_sp<SkSurface> surf = std::move(rasterTask.surf);
      if (!surf)
      {
        surf = rasterSurface(context(), rasterTask.width, rasterTask.height);
        auto canvas = surf->getCanvas();
        canvas->clear(rasterTask.bgColor);
      }
      ASSERT(surf);
      auto canvas = surf->getCanvas();
      for (const auto& b : rasterTask.where)
      {
        const auto rasterRect = b.src.map(rasterTask.matrix);
        canvas->save();
        canvas->translate(b.dst.x - rasterRect.x(), b.dst.y - rasterRect.y());
        canvas->concat(toSkMatrix(rasterTask.matrix));
        canvas->clipRect(SkRect::MakeXYWH(b.src.x(), b.src.y(), b.src.width(), b.src.height()));
        canvas->clear(rasterTask.bgColor);
        rasterTask.picture->playback(canvas);
        canvas->restore();
      }
      return RasterManager::RasterResult(nullptr, std::move(surf), -1);
    });
  add([task]() { (*task)(); });
  return task->get_future();
}
} // namespace VGG::layer
