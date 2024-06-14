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
#include "RasterTask.hpp"
#include "VSkia.hpp"
#include "Renderer.hpp"
#include "TileIterator.hpp"
#include "RasterNodeImpl.hpp"
#include "SimpleRasterExecutor.hpp"

#include "Layer/RasterManager.hpp"
#include "Layer/Raster.hpp"

#include <core/SkSurface.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>

namespace
{
using namespace VGG::layer;
using namespace VGG;
[[maybe_unused]] inline std::unique_ptr<VGG::layer::RasterManager::RasterTask> createRasterTask(
  const std::vector<VGG::Bounds>& bounds,
  const Bounds&                   viewportBounds,
  const glm::mat3&                rasterMatrix,
  const glm::mat3&                localMatrix)
{
  std::vector<SurfaceTask::Where> where;
  where.reserve(bounds.size());
  const auto inversedMatrix = glm::inverse(localMatrix);                // TODO:: optimized
  const auto viewportInWorldSpace = viewportBounds.map(inversedMatrix); // TODO:: optimized

  const auto mat = localMatrix * rasterMatrix; // TODO:: optimized
  for (const auto& dr : bounds)
  {
    const auto worldRect = dr.intersectAs(viewportInWorldSpace);
    if (worldRect.valid() == false)
      continue;
    const auto rbx = worldRect.x();
    const auto rby = worldRect.y();
    const auto devicePos = mat * glm::vec3(rbx, rby, 1.0f); // TODO:: optimized
    where.push_back(
      SurfaceTask::Where{ .dst = { (int)devicePos.x, (int)devicePos.y }, .src = worldRect });
  }
  return std::make_unique<SurfaceTask>(
    0,
    SK_ColorWHITE,
    (int)viewportBounds.width(),
    (int)viewportBounds.height(),
    rasterMatrix,
    std::move(where),
    nullptr,
    nullptr);
}

[[maybe_unused]] inline std::vector<std::unique_ptr<TileTask>> createTileTask(
  RasterManager*                  mgr,
  const std::vector<VGG::Bounds>& bounds,
  const glm::mat3&                rasterMatrix,
  const glm::mat3&                localMatrix)
{
  std::unordered_map<int, std::vector<TileTask::Where>>
    tileDamage; // tile_index -> tile_damage regions
  for (const auto& damage : bounds)
  {
    TileIterator iter(
      toSkRect(damage),
      mgr->width(),
      mgr->height(),
      toSkRect(mgr->bounds().toFloatBounds()));
    while (auto tile = iter.next())
    {
      std::vector<TileTask::Where> where;
      where.reserve(5);
      const int index = tile->first + tile->second * mgr->tileXCount();
      Bounds    tileBounds{ (float)tile->first * mgr->width(),
                         (float)tile->first * mgr->height(),
                         (float)mgr->width(),
                         (float)mgr->height() };
      if (auto isectBounds = tileBounds.intersectAs(damage); isectBounds.valid())
      {
        where.push_back(TileTask::Where{ .dst = { (int)isectBounds.x(), (int)isectBounds.y() },
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

  std::vector<std::unique_ptr<TileTask>> tasks;
  tasks.reserve(tileDamage.size());
  for (const auto& [k, v] : tileDamage)
  {
    tasks.push_back(
      std::make_unique<TileTask>(mgr, k, std::move(v), rasterMatrix, nullptr, nullptr));
  }
  return tasks;
}

enum EMatrixChanged
{
  TRANSLATE = 1,
  SCALE = 2,
};

[[maybe_unused]] inline uint8_t changeReason(glm::mat3& prevMatrix, const glm::mat3& newMatrix)
{
  uint8_t changed = 0;
  if (prevMatrix[0][0] != newMatrix[0][0] || prevMatrix[1][1] != newMatrix[1][1])
  {
    changed |= EMatrixChanged::SCALE;
  }
  else if (prevMatrix[2][0] != newMatrix[2][0] || prevMatrix[2][1] != newMatrix[2][1])
  {
    changed |= EMatrixChanged::TRANSLATE;
  }
  return changed;
}
} // namespace

namespace VGG::layer
{

RasterNodeImpl::RasterNodeImpl(
  VRefCnt*                       cnt,
  RasterManager::RasterExecutor* executor,
  Ref<Viewport>                  viewport,
  Ref<ZoomerNode>                zoomer,
  Ref<RenderNode>                child)
  : RasterNode(
      cnt,
      static_cast<SimpleRasterExecutor*>(executor)->context(),
      executor,
      std::move(viewport),
      std::move(zoomer),
      std::move(child))
{
#ifdef VGG_LAYER_DEBUG
  dbgInfo = "DamageRedrawNode";
#endif
}

void RasterNodeImpl::render(Renderer* renderer)
{
  auto c = getChild();
  ASSERT(c);
  if (!c->picture())
  {
    TransformEffectNode::render(renderer);
  }
  else
  {
    auto canvas = renderer->canvas();
    ASSERT(canvas);
    canvas->save();

    if (true)
    {
      if (auto res = m_rasterMananger.query(0); res)
      {
        canvas->drawImage(res->surf->makeImageSnapshot(), 0, 0);
      }
    }
    else
    {
      canvas->concat(toSkMatrix(getLocalMatrix()));
    }

    canvas->restore();
  }
}

#ifdef VGG_LAYER_DEBUG
void RasterNodeImpl::debug(Renderer* render)
{
  auto c = getChild();
  ASSERT(c);
  if (c)
  {
    auto                z = getTransform();
    SkAutoCanvasRestore acr(render->canvas(), true);
    render->canvas()->concat(toSkMatrix(z->getMatrix()));
    c->debug(render);
  }
}
#endif

sk_sp<SkSurface> g_surface;

SkSurface* rasterSurface(GrRecordingContext* context, int w, int h)
{
  ASSERT(w > 0 && h > 0);
  if (!g_surface || g_surface->width() != w || g_surface->height() != h)
  {
    auto info = SkImageInfo::MakeN32Premul(w, h);
    g_surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
    if (!g_surface)
    {
      return nullptr;
    }
  }
  return g_surface.get();
}

void blit(GrRecordingContext* context, SkSurface* dst, SkSurface* src, int x, int y)
{
  auto dstCanvas = dst->getCanvas();
  dstCanvas->drawImage(src->makeImageSnapshot(), x, y);
}

std::vector<Bounds> mergeBounds(std::vector<Bounds> bounds)
{
  std::vector<Bounds> merged;
  std::sort(
    bounds.begin(),
    bounds.end(),
    [](const Bounds& a, const Bounds& b)
    {
      const auto aTopLeft = a.topLeft();
      const auto bTopLeft = b.topLeft();
      return aTopLeft.y < bTopLeft.y || (aTopLeft.y == bTopLeft.y && aTopLeft.x < bTopLeft.x);
    });
  for (auto& b : bounds)
  {
    if (merged.empty())
    {
      merged.push_back(b);
    }
    else
    {
      auto&       last = merged.back();
      const auto& br = last.bottomRight();
      const auto& tl = b.topLeft();

      if (br.x >= tl.x && br.y >= tl.y)
      {
        last.unionWith(b);
      }
      else
      {
        merged.push_back(b);
      }
    }
  }
  return merged;
}

void RasterNodeImpl::raster(const std::vector<Bounds>& bounds)
{
  ASSERT(!isInvalid());

  auto c = getChild();
  ASSERT(c);
  auto pic = c->picture();
  ASSERT(pic);

  bool contentChanged = false;
  if (m_pictureUniqueID != pic->uniqueID())
  {
    m_pictureUniqueID = pic->uniqueID();
    contentChanged = true;
    const auto sceneBounds = c->bounds();
    m_rasterMananger = std::make_unique<RasterManager>(2048, 512, sceneBounds, executor());
  }

  if (true)
  {
    const auto                      viewportBounds = viewport()->bounds();
    std::vector<SurfaceTask::Where> where;
    where.reserve(bounds.size());
    for (const auto& dr : bounds)
    {
      const auto rasterRect = dr.intersectAs(viewportBounds);
      if (rasterRect.valid() == false)
        continue;
      const auto rbx = rasterRect.x();
      const auto rby = rasterRect.y();
      const auto worldRect = rasterRect.map(getTransform()->getInversedMatrix());
      {
        where.push_back(SurfaceTask::Where{ .dst = { (int)rbx, (int)rby }, .src = worldRect });
      }
    }
    sk_sp<SkSurface> surf;
    if (auto res = m_rasterMananger->query(0); res)
    {
      surf = std::move(res->surf);
    }

    m_rasterMananger->raster(std::make_unique<SurfaceTask>(
      0,
      SK_ColorWHITE,
      (int)viewportBounds.width(),
      (int)viewportBounds.height(),
      getRasterMatrix(),
      std::move(where),
      sk_ref_sp(pic),
      std::move(surf)));
  }
  else
  {
    const auto reason = changeReason(m_prevMatrix, getTransform()->getMatrix());
    if (!bounds.empty() || reason & (EMatrixChanged::SCALE) || contentChanged)
    {
      std::vector<std::unique_ptr<TileTask>> tasks;
      if (contentChanged)
      {
        tasks = createTileTask(m_rasterMananger.get(), bounds, getRasterMatrix(), getLocalMatrix());
      }
      else
      {
        // if content is change, the damage region is forced to be the whole scene, since the scene
        // change cannot emit the correct damage region so far. damage region emit could be unified in
        // the future.
        tasks = createTileTask(
          m_rasterMananger.get(),
          { c->bounds() },
          getRasterMatrix(),
          getLocalMatrix());
      }
      for (auto& task : tasks)
      {
        task->picture = sk_ref_sp(pic);
        if (auto res = m_rasterMananger->query(task->index()); res)
        {
          task->surf = std::move(res->surf);
        }
        m_rasterMananger->raster(std::move(task));
      }
    }
  }
}

Bounds RasterNodeImpl::onRevalidate(Revalidation* inv, const glm::mat3& ctm)
{
  return RasterNode::onRevalidate(inv, ctm);
  // Bounds bounds;
  // if (getTransform())
  // {
  //   getTransform()->revalidate();
  // }
  // if (getChild())
  // {
  //   if (getTransform())
  //   {
  //     const auto matrix = getTransform()->getMatrix();
  //     bounds = getChild()->revalidate(inv, ctm);
  //     bounds = bounds.map(matrix);
  //   }
  //   else
  //   {
  //     bounds = getChild()->revalidate(inv, ctm);
  //   }
  // }
  //
  // if (m_viewport)
  //   m_viewport->revalidate();
  //
  // auto t = getTransform();
  // if (t)
  // {
  //   const auto& totalMatrix = t->getMatrix();
  //   m_localMatrix[2][0] = totalMatrix[2][0];
  //   m_localMatrix[2][1] = totalMatrix[2][1];
  //   m_rasterMatrix = totalMatrix;
  //   m_rasterMatrix[2][0] = 0.0f;
  //   m_rasterMatrix[2][1] = 0.0f;
  // }
  // return bounds;
}

namespace raster
{
Ref<RasterNode> make(
  RasterManager::RasterExecutor* executor,
  Ref<Viewport>                  viewport,
  Ref<ZoomerNode>                zoomer,
  Ref<RenderNode>                child)
{
  return RasterNodeImpl::Make(executor, std::move(viewport), std::move(zoomer), std::move(child));
}
} // namespace raster
} // namespace VGG::layer
