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
#include "Renderer.hpp"
#include "RasterNodeImpl.hpp"

#include "Layer/RasterManager.hpp"
#include "Layer/Raster.hpp"

#include <core/SkSurface.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>

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
  , m_rasterMananger(1024, 1024, Boundsi(0, 0, 1024, 1024), executor)
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
    {
      if (auto res = m_rasterMananger.query(0); res)
      {
        canvas->drawImage(res->surf->makeImageSnapshot(), 0, 0);
      }
    }
    // canvas->drawImage(m_gpuSurface->makeImageSnapshot(), 0, 0);
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

  if (m_viewportBounds != viewport()->bounds())
  {
    m_viewportBounds = viewport()->bounds();
    m_gpuSurface = SkSurfaces::RenderTarget(
      device(),
      skgpu::Budgeted::kYes,
      SkImageInfo::MakeN32Premul(m_viewportBounds.width(), m_viewportBounds.height()));
  }

  auto c = getChild();
  ASSERT(c);
  auto pic = c->picture();
  ASSERT(pic);

  std::vector<SurfaceTask::Where> where;
  where.reserve(bounds.size());
  for (const auto& dr : bounds)
  {
    const auto rasterRect = dr.intersectAs(m_viewportBounds);
    if (rasterRect.valid() == false)
      continue;
    const auto rbx = rasterRect.x();
    const auto rby = rasterRect.y();
    // const auto rbw = rasterRect.width();
    // const auto rbh = rasterRect.height();
    const auto worldRect = rasterRect.map(getTransform()->getInversedMatrix());
    {
      where.push_back(SurfaceTask::Where{ .dst = { (int)rbx, (int)rby }, .src = worldRect });
    }
  }
  sk_sp<SkSurface> surf;
  if (auto res = m_rasterMananger.query(0); res)
  {
    surf = std::move(res->surf);
  }

  m_rasterMananger.raster(std::make_unique<SurfaceTask>(
    0,
    SK_ColorWHITE,
    (int)m_viewportBounds.width(),
    (int)m_viewportBounds.height(),
    getRasterMatrix(),
    std::move(where),
    sk_ref_sp(pic),
    std::move(surf)));
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
