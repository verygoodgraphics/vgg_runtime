/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
#include "Executor.hpp"
#include "VSkia.hpp"
#include "TileIterator.hpp"
#include "Layer/Core/VBounds.hpp"
#include "Layer/LRUCache.hpp"

#include <core/SkImage.h>
#include <core/SkSurface.h>
#include <core/SkCanvas.h>
#include <core/SkPicture.h>
#include <queue>
#include <future>

class GrRecordingContext;
namespace VGG::layer
{

class RasterExecutor;

class RasterManager
{
public:
  using Key = uint64_t;
  class RasterResult
  {
  public:
    using Future = std::future<RasterResult>;
    RasterResult() = default;
    RasterResult(RasterManager* mgr, sk_sp<SkSurface> surf, Key index)
      : surf(std::move(surf))
      , m_mgr(mgr)
      , m_index(index)
    {
    }

    std::pair<int, int> pos() const
    {
      return { m_rx, m_ry };
    }

    Key index() const
    {
      return m_index;
    }

    sk_sp<SkSurface> surf = nullptr;

  private:
    RasterManager* m_mgr = nullptr;
    int            m_rx = 0, m_ry = 0;
    Key            m_index = 0;
  };

  class RasterTask
  {
  public:
    RasterTask(Key index)
      : m_index(index)
    {
    }
    Key index() const
    {
      return m_index;
    }
    virtual RasterResult execute(GrRecordingContext* context) = 0;
    virtual ~RasterTask() = default;

  private:
    Key m_index;
  };

  class RasterExecutor : public Executor
  {
  public:
    virtual RasterManager::RasterResult::Future addRasterTask(
      std::unique_ptr<RasterManager::RasterTask> task) = 0;
  };

  RasterManager(RasterExecutor* executor)
    : m_executor(executor)
    , m_cache(40)
  {
  }

  RasterManager(const RasterManager&) = delete;
  RasterManager& operator=(const RasterManager&) = delete;

  void updateDamage(
    int                 tw,
    int                 th,
    std::vector<Bounds> damageBounds,
    const glm::mat3&    rasterMatrix,
    const Bounds&       worldBounds,
    sk_sp<SkPicture>    pic);

  void update(std::vector<std::unique_ptr<RasterTask>> tasks);

  void appendRasterTask(std::unique_ptr<RasterTask> task);

  RasterResult syncExecuteRasterTask(std::unique_ptr<RasterTask> task);

  void query(const std::vector<Key>& query, std::vector<RasterResult>& result);
  std::optional<RasterResult> query(Key index);

private:
  using ResultCache = LRUCache<Key, RasterResult>;
  using TaskQueue = std::queue<std::pair<Key, std::future<RasterResult>>>;
  void            wait();
  RasterExecutor* m_executor;
  ResultCache     m_cache;
  TaskQueue       m_tasks;
};

} // namespace VGG::layer
