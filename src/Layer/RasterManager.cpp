#include "RasterManager.hpp"
#include "TileIterator.hpp"
#include "RasterTask.hpp"

#include "core/SkSurface.h"

#include <core/SkColor.h>
#include <optional>
#include <vector>
#include <future>

namespace VGG::layer
{

void RasterManager::query(const std::vector<Key>& query, std::vector<RasterResult>& result)
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

std::optional<RasterManager::RasterResult> RasterManager::query(Key index)
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
  int                 tw,
  int                 th,
  std::vector<Bounds> rasterDamageBounds,
  const glm::mat3&    rasterMatrix,
  const Bounds&       worldBounds,
  sk_sp<SkPicture>    pic)
{
  const auto rasterBounds = worldBounds.map(rasterMatrix);
  std::unordered_map<Key, std::vector<TileTask::Where>>
    tileDamage; // tile_index -> tile_damage regions
  for (const auto& damage : rasterDamageBounds)
  {
    TileIter iter(damage, tw, th, rasterBounds);
    while (auto tile = iter.next())
    {
      std::vector<TileTask::Where> where;
      where.reserve(5);
      const auto key = tile->key();
      const auto tileBounds = tile->bounds().toFloatBounds();
      if (auto isectBounds = tileBounds.intersectAs(damage); isectBounds.valid())
      {
        where.push_back(TileTask::Where{ .dst = { (int)isectBounds.x() - tile->topLeft().x,
                                                  (int)isectBounds.y() - tile->topLeft().y },
                                         .src = isectBounds });
      }
      if (auto it = tileDamage.find(key); it != tileDamage.end())
      {
        it->second.insert(
          it->second.end(),
          std::move_iterator(where.begin()),
          std::move_iterator(where.end()));
      }
      else
      {
        tileDamage.insert({ key, std::move(where) });
      }
    }
  }

  for (const auto& [k, v] : tileDamage)
  {
    sk_sp<SkSurface> surf;
    if (auto cache = query(k); cache)
    {
      surf = std::move(cache->surf);
      auto task = std::make_unique<TileTask>(
        this,
        k,
        tw,
        th,
        SK_ColorTRANSPARENT,
        std::move(v),
        rasterMatrix,
        pic,
        std::move(surf));
      appendRasterTask(std::move(task));
    }
  }
}

void RasterManager::update(std::vector<std::unique_ptr<RasterTask>> tasks)
{
  m_cache.purge();
  for (auto& task : tasks)
  {
    appendRasterTask(std::move(task));
  }
}

} // namespace VGG::layer
