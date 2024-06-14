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

void RasterManager::raster(std::unique_ptr<RasterTask> task)
{
  m_tasks.push({ task->index(), m_executor->addRasterTask(std::move(task)) });
}

RasterManager::RasterResult::Future SimpleRasterExecutor::addRasterTask(
  std::unique_ptr<RasterManager::RasterTask> rasterTask)
{
  using RR = RasterManager::RasterResult;
  const auto task =
    std::make_shared<std::packaged_task<RR()>>([&]() { return rasterTask->execute(context()); });
  add([task]() { (*task)(); });
  return task->get_future();
}
} // namespace VGG::layer
