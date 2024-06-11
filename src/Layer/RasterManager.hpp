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

class RasterManager : public VNode
{
public:
  class RasterTask
  {
  public:
    RasterTask(
      RasterManager*   mgr,
      int              index,
      sk_sp<SkPicture> picture,
      const Boundsi&   damageBounds,
      sk_sp<SkSurface> surf = nullptr)
      : damageBounds(damageBounds)
      , picture(std::move(picture))
      , surf(std::move(surf))
      , mgr(mgr)
      , index(-1)
    {
    }
    Boundsi          damageBounds;
    sk_sp<SkPicture> picture;
    sk_sp<SkSurface> surf; // optional, null indicates this is a new surface
    RasterManager*   mgr;
    int              index;
  };

  class RasterResult
  {
  public:
    RasterResult(RasterManager* mgr, sk_sp<SkSurface> surf, int index)
      : m_mgr(mgr)
      , m_surf(std::move(surf))
      , m_index(index)
    {
    }

    std::pair<int, int> pos() const
    {
      return { m_rx, m_ry };
    }

    sk_sp<SkImage> image() const
    {
      return m_surf->makeImageSnapshot();
    }

  private:
    RasterManager*   m_mgr;
    sk_sp<SkSurface> m_surf;
    int              m_rx, m_ry;
    int              m_index;
  };

  RasterManager(
    VRefCnt*        cnt,
    int             tileWidth,
    int             tileHeight,
    const Bounds&   bounds,
    RasterExecutor* executor)
    : VNode(cnt)
    , m_executor(executor)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight)
    , m_cache(40)
  {
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

  void raster(std::vector<RasterTask> tasks);

  void query(const std::vector<int>& query, std::vector<RasterResult>& result);

private:
  RasterExecutor* m_executor;
  int             m_tileWidth, m_tileHeight;
  Boundsi         m_bounds;
  using ResultCache = LRUCache<int, std::future<RasterResult>>;
  ResultCache m_cache;
};

class RasterExecutor : public Executor
{
public:
  virtual std::future<RasterManager::RasterResult> addRasterTask(
    RasterManager::RasterTask task) = 0;
};

class SimpleRasterExecutor : public RasterExecutor
{
public:
  using Future = std::future<RasterManager::RasterResult>;
  SimpleRasterExecutor(GrRecordingContext* context)
    : m_context(context)
  {
  }

  Future addRasterTask(RasterManager::RasterTask rasterTask) override;

  void add(Task task) override
  {
    task();
  }

protected:
  GrRecordingContext* context()
  {
    return m_context;
  }

private:
  GrRecordingContext* m_context;
};

} // namespace VGG::layer
