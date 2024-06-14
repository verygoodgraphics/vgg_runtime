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
#include "Layer/Core/VBounds.hpp"
#include "Layer/Core/VNode.hpp"
#include "Layer/LRUCache.hpp"

#include <core/SkImage.h>
#include <core/SkSurface.h>
#include <core/SkCanvas.h>
#include <core/SkPicture.h>

#include <future>

class GrRecordingContext;
namespace VGG::layer
{

class RasterExecutor;

class RasterManager
{
public:
  class RasterResult
  {
  public:
    using Future = std::future<RasterResult>;
    RasterResult() = default;
    RasterResult(RasterManager* mgr, sk_sp<SkSurface> surf, int index)
      : surf(std::move(surf))
      , m_mgr(mgr)
      , m_index(index)
    {
    }

    std::pair<int, int> pos() const
    {
      return { m_rx, m_ry };
    }

    sk_sp<SkSurface> surf = nullptr;

  private:
    RasterManager* m_mgr = nullptr;
    int            m_rx = 0, m_ry = 0;
    int            m_index = -1;
  };

  class RasterTask
  {
  public:
    RasterTask(int index)
      : m_index(index)
    {
    }
    int index() const
    {
      return m_index;
    }
    virtual RasterResult execute(GrRecordingContext* context) = 0;

  private:
    int m_index;
  };

  class RasterExecutor : public Executor
  {
  public:
    virtual RasterManager::RasterResult::Future addRasterTask(
      std::unique_ptr<RasterManager::RasterTask> task) = 0;
  };

  RasterManager(int tileWidth, int tileHeight, const Boundsi& bounds, RasterExecutor* executor)
    : m_executor(executor)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight)
    , m_bounds(bounds)
    , m_cache(40)
  {
  }

  RasterManager(const RasterManager&) = delete;
  RasterManager& operator=(const RasterManager&) = delete;

  std::pair<int, int> size() const
  {
    return { m_tileWidth, m_tileHeight };
  }

  int width() const
  {
    return m_tileWidth;
  }

  int height() const
  {
    return m_tileHeight;
  }

  const Boundsi& bounds() const
  {
    return m_bounds;
  }

  void raster(std::unique_ptr<RasterTask> task);

  void query(const std::vector<int>& query, std::vector<RasterResult>& result);

  std::optional<RasterResult> query(int index);

private:
  using ResultCache = LRUCache<int, RasterResult>;
  using TaskQueue = std::queue<std::pair<int, std::future<RasterResult>>>;
  void            wait();
  RasterExecutor* m_executor;
  int             m_tileWidth, m_tileHeight;
  Boundsi         m_bounds;
  ResultCache     m_cache;
  TaskQueue       m_tasks;
};

class SimpleRasterExecutor : public RasterManager::RasterExecutor
{
public:
  SimpleRasterExecutor(GrRecordingContext* context)
    : m_context(context)
  {
  }

  RasterManager::RasterResult::Future addRasterTask(
    std::unique_ptr<RasterManager::RasterTask> rasterTask) override;

  void add(Task task) override
  {
    task();
  }

  GrRecordingContext* context()
  {
    return m_context;
  }

private:
  GrRecordingContext* m_context;
  SimpleRasterExecutor(SimpleRasterExecutor&&) = delete;
  SimpleRasterExecutor&& operator=(SimpleRasterExecutor&&) = delete;
};

} // namespace VGG::layer
