#include "RasterManager.hpp"
#include "TileIterator.hpp"
#include "RasterTask.hpp"
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

RasterManager::RasterResult RasterManager::syncExecuteRasterTask(std::unique_ptr<RasterTask> task)
{
  auto res = m_executor->addRasterTask(std::move(task)).get();
  return *m_cache.insertOrUpdate(res.index(), std::move(res));
}

void RasterManager::appendRasterTask(std::unique_ptr<RasterTask> task)
{
  if (task)
  {
    m_tasks.push({ task->index(), m_executor->addRasterTask(std::move(task)) });
  }
}

void RasterManager::updateDamage(
  std::vector<Bounds> rasterDamageBounds,
  const glm::mat3&    rasterMatrix,
  const Bounds&       worldBounds,
  sk_sp<SkPicture>    pic)
{
  const auto rasterBounds = worldBounds.map(rasterMatrix);
  std::unordered_map<int, std::vector<TileTask::Where>>
    tileDamage; // tile_index -> tile_damage regions
  for (const auto& damage : rasterDamageBounds)
  {
    TileIterator iter(toSkRect(damage), this->width(), this->height(), toSkRect(rasterBounds));

    while (auto tile = iter.next())
    {
      std::vector<TileTask::Where> where;
      where.reserve(5);
      const int  index = iter.index(tile->first, tile->second);
      const auto tileBounds = iter.bounds(tile->first, tile->second).toFloatBounds();
      const auto [x, y] = iter.pos(tile->first, tile->second);
      if (auto isectBounds = tileBounds.intersectAs(damage); isectBounds.valid())
      {
        where.push_back(
          TileTask::Where{ .dst = { (int)isectBounds.x() - x, (int)isectBounds.y() - y },
                           .src = isectBounds });
      }
      if (auto it = tileDamage.find(index); it != tileDamage.end())
      {
        it->second.insert(
          it->second.end(),
          std::move_iterator(where.begin()),
          std::move_iterator(where.end()));
      }
      else
      {
        tileDamage.insert({ index, std::move(where) });
      }
    }
  }

  for (const auto& [k, v] : tileDamage)
  {
    sk_sp<SkSurface> surf;
    if (auto cache = query(k); cache)
    {
      surf = std::move(cache->surf);
      auto task =
        std::make_unique<TileTask>(this, k, std::move(v), rasterMatrix, pic, std::move(surf));
      appendRasterTask(std::move(task));
    }
  }
}

void RasterManager::update(std::vector<std::unique_ptr<RasterTask>> tasks)
{
  for (auto& task : tasks)
  {
    appendRasterTask(std::move(task));
  }
}

} // namespace VGG::layer
