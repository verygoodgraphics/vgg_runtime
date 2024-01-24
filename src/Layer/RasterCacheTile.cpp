/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "Layer/Core/RasterCacheTile.hpp"
#include "Layer/Core/RasterCache.hpp"

#include "core/SkCanvas.h"
#include "core/SkImage.h"
#include "core/SkPicture.h"
#include "core/SkSurface.h"
#include <gpu/ganesh/SkSurfaceGanesh.h>

namespace
{

std::vector<Rasterizer::Tile> rasterOrGetTile(
  SkSurface*                                       surface,
  const Rasterizer::RasterContext&                 rasterContext,
  const SkMatrix&                                  rasterMatrix,
  std::vector<RasterCacheTile::TileMap::iterator>& hitTiles)
{
  ASSERT(surface);
  for (auto& t : hitTiles)
  {
    auto& tile = *t;
    if (tile.image)
      continue;
    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->save();
    canvas->translate(-tile.rect.x(), -tile.rect.y());
    canvas->concat(rasterMatrix);
    canvas->drawPicture(rasterContext.picture);
    canvas->restore();
    tile.image = surface->makeImageSnapshot();
  }

  std::vector<Rasterizer::Tile> tiles;
  for (auto& t : hitTiles)
  {
    tiles.push_back(*t);
  }
  return tiles;
}
std::vector<RasterCacheTile::TileMap::iterator> hitTile(
  RasterCacheTile::TileMap& tileCache,
  const SkRect&             viewport,
  const SkMatrix&           transform)
{
  if (tileCache.empty())
    return {};
  std::vector<RasterCacheTile::TileMap::iterator> res;
  res.reserve(4);
  DEBUG("Hit viewport: %f %f", viewport.width(), viewport.height());
  for (auto it = tileCache.begin(); it != tileCache.end(); ++it)
  {
    auto& tile = *it;
    auto  r = transform.mapRect(tile.rect);
    if (r.intersects(viewport))
    {
      DEBUG(
        "Hit tile: %f %f [%f %f] %d",
        r.top(),
        r.left(),
        tile.rect.top(),
        tile.rect.left(),
        tile.image == nullptr ? 0 : 1);
      res.push_back(it);
    }
  }
  return res;
}

void calculateTiles(RasterCacheTile::TileMap& tileCache, const SkRect& content, float tw, float th)
{
  tileCache.clear();
  auto x = content.x();
  auto y = content.y();
  auto w = content.width();
  auto h = content.height();
  auto tileX = x;
  auto tileY = y;
  auto tileW = tw;
  auto tileH = th;

  while (tileX < x + w)
  {
    while (tileY < y + h)
    {
      auto tile = SkRect::MakeXYWH(tileX, tileY, tileW, tileH);
      tileCache.push_back({ nullptr, tile });
      tileY += tileH;
    }
    tileY = y;
    tileX += tileW;
  }
}
} // namespace

namespace VGG::layer
{

void RasterCacheTile::revalidate(
  LevelCache&     levelCache,
  const SkMatrix& transform,
  const SkMatrix& localMatrix,
  const SkRect&   bound)
{
  if (levelCache.invalid)
  {
    levelCache.rasterMatrix = transform;
    levelCache.rasterMatrix[SkMatrix::kMTransX] = 0;
    levelCache.rasterMatrix[SkMatrix::kMTransY] = 0;
    levelCache.rasterMatrix.postConcat(localMatrix);
    const auto contentRect = levelCache.rasterMatrix.mapRect(bound);
    calculateTiles(levelCache.tileCache, contentRect, m_tileWidth, m_tileHeight);
    levelCache.invalid = false;
  }
}

SkSurface* RasterCacheTile::rasterSurface(GrRecordingContext* context)
{
  if (!m_surface)
  {
    ASSERT(m_tileWidth > 0 && m_tileHeight > 0);
    auto info = SkImageInfo::MakeN32Premul(m_tileWidth, m_tileHeight);
    m_surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
    if (!m_surface)
    {
      return nullptr;
    }
  }
  return m_surface.get();
}

std::tuple<uint32_t, std::vector<Rasterizer::Tile>, SkMatrix> RasterCacheTile::onRevalidateRaster(
  uint32_t             reason,
  GrRecordingContext*  context,
  int                  lod,
  const SkRect&        clipRect,
  const RasterContext& rasterContext,
  void*                userData)
{
  DEBUG("reason: %s", printReason(reason).c_str());

  auto       skv = clipRect;
  const auto localMatrix = rasterContext.localMatrix;
  auto       hitMatrix = SkMatrix::I();
  hitMatrix[SkMatrix::kMTransX] = rasterContext.globalMatrix.getTranslateX();
  hitMatrix[SkMatrix::kMTransY] = rasterContext.globalMatrix.getTranslateY();

  std::vector<TileMap::iterator> hitTiles;

  ASSERT(m_cacheStack.size() > 0);
  ASSERT(lod < int(m_cacheStack.size() - 1));

  size_t cacheIndex = lod < 0 ? m_cacheStack.size() - 1 : lod;
  auto&  currentLevelCache = m_cacheStack[cacheIndex];

  if (reason & CONTENT)
  {
    invalidateContent();
    revalidate(currentLevelCache, rasterContext.globalMatrix, localMatrix, *rasterContext.bound);
    hitTiles = hitTile(currentLevelCache.tileCache, skv, hitMatrix);
    if (hitTiles.empty())
    {
      return { reason, {}, hitMatrix };
    }
    auto surface = rasterSurface(context);
    auto tiles = rasterOrGetTile(surface, rasterContext, currentLevelCache.rasterMatrix, hitTiles);
    return { reason, std::move(tiles), hitMatrix };
  }

  if ((reason & ZOOM_TRANSLATION) && !(reason & ZOOM_SCALE)) // most case
  {
    DEBUG("translation");
    revalidate(currentLevelCache, rasterContext.globalMatrix, localMatrix, *rasterContext.bound);
    hitTiles = hitTile(currentLevelCache.tileCache, skv, hitMatrix);
    if (hitTiles.empty())
    {
      return { reason, {}, hitMatrix };
    }
  }

  if (reason & ZOOM_SCALE)
  {
    if (lod < 0)
    {
      currentLevelCache.invalid = true;
    }
    revalidate(currentLevelCache, rasterContext.globalMatrix, localMatrix, *rasterContext.bound);
    hitTiles = hitTile(currentLevelCache.tileCache, skv, hitMatrix);
  }

  if (hitTiles.empty())
  {
    DEBUG("no tile hit");
    return { reason, {}, hitMatrix };
  }
  auto surface = rasterSurface(context);
  auto tiles = rasterOrGetTile(surface, rasterContext, currentLevelCache.rasterMatrix, hitTiles);
  return { reason, std::move(tiles), hitMatrix };
}
RasterCacheTile::RasterCacheTile(float tw, float th)
  : m_tileWidth(tw)
  , m_tileHeight(th)
{
  m_cacheStack.resize(Zoomer::ZOOM_LEVEL_COUNT + 1);
}

RasterCacheTile::~RasterCacheTile()
{
}

} // namespace VGG::layer
