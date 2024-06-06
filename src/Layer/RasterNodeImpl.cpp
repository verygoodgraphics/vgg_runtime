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
#include "RasterNodeImpl.hpp"

#include "Renderer.hpp"
#include "Layer/Raster.hpp"

#include <core/SkSurface.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>

namespace VGG::layer
{

RasterNodeImpl::RasterNodeImpl(
  VRefCnt*            cnt,
  GrRecordingContext* device,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     child)
  : RasterNode(cnt, device, std::move(viewport), std::move(zoomer), std::move(child))
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
    canvas->drawImage(m_gpuSurface->makeImageSnapshot(), 0, 0);
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

  for (const auto& dr : bounds)
  {
    const auto rasterRect = dr.intersectAs(m_viewportBounds);
    if (rasterRect.valid() == false)
      continue;
    const auto rbx = rasterRect.x();
    const auto rby = rasterRect.y();
    const auto rbw = rasterRect.width();
    const auto rbh = rasterRect.height();

    auto surf = rasterSurface(device(), rbw, rbh);

    // ASSERT(getTransform()->getMatrix() == m_deviceMatrix * m_rasterMatrix);
    ASSERT(surf);
    auto cvs = surf->getCanvas();
    ASSERT(cvs);
    cvs->save();
    cvs->clear(SK_ColorWHITE);
    cvs->translate(-rbx, -rby);
    cvs->concat(toSkMatrix(getTransform()->getMatrix()));
    // cvs->clipRect(SkRect::MakeXYWH(0, 0, rbh, rbw));
    auto c = getChild();
    ASSERT(c);
    auto pic = c->picture();
    ASSERT(pic);
    pic->playback(cvs);
    cvs->restore();

    blit(device(), m_gpuSurface.get(), surf, rbx, rby);
  }
}

namespace raster
{
Ref<RasterNode> make(
  GrRecordingContext* device,
  Ref<Viewport>       viewport,
  Ref<ZoomerNode>     zoomer,
  Ref<RenderNode>     child)
{
  return RasterNodeImpl::Make(device, std::move(viewport), std::move(zoomer), std::move(child));
}
} // namespace raster
} // namespace VGG::layer