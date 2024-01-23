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

namespace VGG::layer
{

void RasterCacheTile::revalidate(
  const SkMatrix& transform,
  const SkMatrix& localMatrix,
  const SkRect&   bound)
{
  m_rasterMatrix = transform;
  m_rasterMatrix[SkMatrix::kMTransX] = 0;
  m_rasterMatrix[SkMatrix::kMTransY] = 0;
  m_rasterMatrix.postConcat(localMatrix);
  const auto contentRect = m_rasterMatrix.mapRect(bound);
  calculateTiles(contentRect);
}

void RasterCacheTile::calculateTiles(const SkRect& content)
{
  m_tileCache.clear();
  auto x = content.x();
  auto y = content.y();
  auto w = content.width();
  auto h = content.height();
  auto tileX = x;
  auto tileY = y;
  auto tileW = m_tileWidth;
  auto tileH = m_tileHeight;

  while (tileX < x + w)
  {
    while (tileY < y + h)
    {
      auto tile = SkRect::MakeXYWH(tileX, tileY, tileW, tileH);
      m_tileCache.push_back({ nullptr, tile });
      tileY += tileH;
    }
    tileY = y;
    tileX += tileW;
  }
}

std::vector<RasterCacheTile::TileMap::iterator> RasterCacheTile::hitTile(
  const SkRect&   viewport,
  const SkMatrix& transform)
{
  if (m_tileCache.empty())
    return {};
  std::vector<TileMap::iterator> res;
  res.reserve(4);
  DEBUG("Hit viewport: %f %f", viewport.width(), viewport.height());
  for (auto it = m_tileCache.begin(); it != m_tileCache.end(); ++it)
  {
    auto& tile = *it;
    auto  r = transform.mapRect(tile.rect);
    if (r.intersects(viewport))
    {
      DEBUG("Hit tile: %f %f [%f %f] ", r.top(), r.left(), tile.rect.top(), tile.rect.left());
      res.push_back(it);
    }
  }
  return res;
}

uint32_t RasterCacheTile::onRaster(
  uint32_t            reason,
  GrRecordingContext* context,
  const SkMatrix*     transform,
  const SkRect&       clipRect,
  SkPicture*          pic,
  const SkRect&       bound,
  const SkMatrix&     mat,
  void*               userData)
{

  auto       skv = clipRect;
  const auto localMatrix = mat;
  m_hitMatrix[SkMatrix::kMTransX] = transform->getTranslateX();
  m_hitMatrix[SkMatrix::kMTransY] = transform->getTranslateY();

  std::vector<TileMap::iterator> hitTiles;

  if ((reason & ZOOM_TRANSLATION) && !(reason & ZOOM_SCALE)) // most case
  {
    DEBUG("ignore zoom translation");
    if (m_tileCache.empty())
    {
      revalidate(*transform, localMatrix, bound);
    }
    hitTiles = hitTile(skv, hitMatrix());
    if (hitTiles.empty())
    {
      m_hitTiles.clear();
      return reason;
    }
  }

  if (reason & ZOOM_SCALE || reason & CONTENT)
  {
    revalidate(*transform, localMatrix, bound);
    hitTiles = hitTile(skv, hitMatrix());
  }

  if (hitTiles.empty())
  {
    DEBUG("no tile hit");
    m_hitTiles.clear();
    return reason;
  }

  sk_sp<SkSurface> surface = nullptr;
  for (auto& t : hitTiles)
  {
    auto& tile = *t;
    if (tile.image)
      continue;
    if (!surface)
    {
      auto info = SkImageInfo::MakeN32Premul(m_tileWidth, m_tileHeight);
      surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
      if (!surface)
      {
        DEBUG("Raster failed");
        return 0;
      }
    }
    auto canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->save();
    canvas->translate(-tile.rect.x(), -tile.rect.y());
    canvas->concat(rasterMatrix());
    canvas->drawPicture(pic);
    canvas->restore();
    tile.image = surface->makeImageSnapshot();
  }
  m_hitTiles.clear();
  for (auto& t : hitTiles)
  {
    m_hitTiles.push_back(*t);
  }
  return reason;
}

} // namespace VGG::layer
