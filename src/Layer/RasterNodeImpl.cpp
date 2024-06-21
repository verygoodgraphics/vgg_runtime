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

#include <core/SkColor.h>
#include <core/SkRefCnt.h>
#include <core/SkSurface.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>

constexpr bool ENABLE_TILE = true;

namespace
{
using namespace VGG::layer;
using namespace VGG;

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

std::pair<int, int> evalTileSize(float w, float h, const Bounds& clipRect)
{
  ASSERT(w > 0 && h > 0);
  int  newTileHeight = 0;
  int  newTileWidth = 0;
  auto length = [](float l, int v, int n)
  {
    constexpr int MAX_TILE_SIZE = 3072;
    constexpr int MIN_TILE_SIZE = 768;
    const int     a = std::max(std::floor(n * l / (v + 10)), 1.f);
    return std::ceil(std::max(std::min(MAX_TILE_SIZE, int(l / a)), MIN_TILE_SIZE));
  };
  newTileWidth = length(w, clipRect.width(), 2);
  newTileHeight = length(h, clipRect.height(), 4);
  auto tileWidth = newTileWidth;
  auto tileHeight = newTileHeight;
  if (w / h > 3)
  {
    tileWidth = newTileHeight;
    tileHeight = newTileWidth;
  }
  tileWidth = w < clipRect.width() ? std::ceil(w) : tileWidth;
  tileHeight = h < clipRect.height() ? std::ceil(h) : tileHeight;
  ASSERT(tileHeight > 0 && tileWidth > 0);
  return { tileWidth, tileHeight };
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
      std::move(child),
      ENABLE_TILE ? EDamageTraitBits::BUBBLE_DAMAGE : 0)
{

  m_rasterMananger = std::make_unique<RasterManager>(executor);
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

    if (!ENABLE_TILE)
    {
      if (auto res = m_rasterMananger->query(0); res)
      {
        canvas->drawImage(res->surf->makeImageSnapshot(), 0, 0);
      }
    }
    else
    {
      TileIter it(viewportBoundsInRasterSpace(), m_tw, m_th, worldBoundsInRasterSpace());
      canvas->concat(toSkMatrix(getLocalMatrix()));
      while (auto tile = it.next())
      {
        const auto key = tile->key();
        if (auto res = m_rasterMananger->query(key); res)
        {
          canvas->drawImage(res->surf->makeImageSnapshot(), tile->topLeft().x, tile->topLeft().y);
        }
        else
        {
          auto t = std::make_unique<TileTask>(
            m_rasterMananger.get(),
            key,
            m_tw,
            m_th,
            SK_ColorTRANSPARENT,
            std::vector{
              TileTask::Where{ .dst = { 0, 0 }, .src = tile->bounds().toFloatBounds() } },
            getRasterMatrix(),
            sk_ref_sp(c->picture()),
            nullptr);
          auto r = m_rasterMananger->syncExecuteRasterTask(std::move(t));
          canvas->drawImage(r.surf->makeImageSnapshot(), tile->topLeft().x, tile->topLeft().y);
        }
      }
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
    {
      auto                z = getTransform();
      SkAutoCanvasRestore acr(render->canvas(), true);
      render->canvas()->concat(toSkMatrix(z->getMatrix()));
      c->debug(render);
    }
    {
      SkAutoCanvasRestore acr(render->canvas(), true);
      TileIter            it(viewportBoundsInRasterSpace(), m_tw, m_th, worldBoundsInRasterSpace());
      render->canvas()->concat(toSkMatrix(getLocalMatrix()));
      SkPaint p;
      p.setColor(SK_ColorBLUE);
      p.setStyle(SkPaint::kStroke_Style);
      while (auto tile = it.next())
      {
        render->canvas()->drawRect(toSkRect(tile->bounds().toFloatBounds()), p);
      }
    }
  }
}
#endif

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
  const auto vb = viewport()->bounds();
  const auto wr = worldBoundsInRasterSpace();
  if (m_viewportBounds != vb || m_rasterBounds != wr)
  {
    m_viewportBounds = vb;
    m_rasterBounds = wr;
    std::tie(m_tw, m_th) = evalTileSize(wr.width(), wr.height(), vb);
  }

  if (!ENABLE_TILE)
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

    m_rasterMananger->appendRasterTask(std::make_unique<SurfaceTask>(
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
    m_prevMatrix = getTransform()->getMatrix();
    if ((reason & EMatrixChanged::SCALE))
    {
      std::vector<std::unique_ptr<RasterManager::RasterTask>> tasks;
      TileIter it(viewportBoundsInRasterSpace(), m_tw, m_th, worldBoundsInRasterSpace());
      while (auto tile = it.next())
      {
        const auto                   k = tile->key();
        std::vector<TileTask::Where> where = {
          TileTask::Where{ .dst = { 0, 0 }, .src = tile->bounds().toFloatBounds() }
        };

        auto task = std::make_unique<TileTask>(
          m_rasterMananger.get(),
          k,
          m_tw,
          m_th,
          SK_ColorTRANSPARENT,
          std::move(where),
          getRasterMatrix(),
          sk_ref_sp(pic),
          nullptr);
        tasks.push_back(std::move(task));
      }

      m_rasterMananger->update(std::move(tasks));
    }
    else if (!bounds.empty())
    {
      auto rasterDamageBounds = bounds;
      for (auto& rb : rasterDamageBounds)
      {
        rb = rb.map(getInversedLocalMatrix()); // Convert to raster space, refactor later
      }
      m_rasterMananger->updateDamage(
        m_tw,
        m_th,
        rasterDamageBounds,
        getRasterMatrix(),
        c->bounds(),
        sk_ref_sp(pic));
    }
    else
    {
    }
  }
}

Bounds RasterNodeImpl::onRevalidate(Revalidation* inv, const glm::mat3& ctm)
{
  const auto bounds = RasterNode::onRevalidate(inv, ctm);
  return bounds;
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
