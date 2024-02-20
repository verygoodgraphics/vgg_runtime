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

#include "Layer/LRUCache.hpp"
#include "core/SkCanvas.h"
#include "core/SkImage.h"
#include "core/SkPicture.h"
#include "core/SkSurface.h"
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <optional>

namespace
{

struct TileIterator
{
  TileIterator(const SkRect& clip, int tileW, int tileH, const SkRect& bound)
    : tileWidth(tileW)
    , tileHeight(tileH)
  {
    auto clipRect = clip.makeOffset(-bound.x(), -bound.y());
    beginX = std::max(0, int((clipRect.x()) / tileW));
    beginY = std::max(0, int((clipRect.y()) / tileH));
    endX = std::min(std::ceil(bound.width() / tileW), std::ceil(clipRect.right() / tileW));
    endY = std::min(std::ceil(bound.height() / tileH), std::ceil(clipRect.bottom() / tileH));
    m_x = beginX;
    m_y = beginY;
    column = std::ceil(bound.width() / tileW);
  }

  TileIterator()
    : tileWidth(0)
    , tileHeight(0)
    , beginX(0)
    , beginY(0)
    , endX(0)
    , endY(0)
    , column(0)
    , m_x(0)
    , m_y(0)
  {
  }

  bool valid() const
  {
    return m_x < endX && m_y < endY;
  }

  std::optional<std::pair<int, int>> next()
  {
    if (m_x >= endX)
    {
      m_x = beginX;
      m_y++;
    }
    if (m_y >= endY)
    {
      return std::nullopt;
    }
    auto r = std::make_pair(m_x, m_y);
    m_x++;
    return r;
  }

  bool contains(int x, int y) const
  {
    return x >= beginX && x < endX && y >= beginY && y < endY;
  }

  bool operator==(const TileIterator& other) const
  {
    return tileWidth == other.tileWidth && tileHeight == other.tileHeight &&
           beginX == other.beginX && beginY == other.beginY && endX == other.endX &&
           endY == other.endY;
  }

  int tileWidth, tileHeight;
  int beginX, beginY;
  int endX, endY;
  int column;

private:
  int m_x, m_y;
};

sk_sp<SkImage> rasterTile(
  SkSurface*      surface,
  SkPicture*      picture,
  const SkMatrix& rasterMatrix,
  const SkRect&   rect)
{
  ASSERT(surface);
  auto canvas = surface->getCanvas();
  canvas->clear(SK_ColorTRANSPARENT);
  canvas->save();
  canvas->translate(-rect.x(), -rect.y());
  canvas->concat(rasterMatrix);
  canvas->drawPicture(picture);
  canvas->restore();
  return surface->makeImageSnapshot();
}

std::vector<Rasterizer::Tile> rasterOrGetTiles(
  SkSurface*                       surface,
  const Rasterizer::RasterContext& rasterContext,
  RasterCacheTile::CacheState&     cache,
  TileIterator                     iter,
  TileIterator                     rasterIter)
{
  std::vector<Rasterizer::Tile> tiles;
  tiles.reserve(8);
  while (auto tile = iter.next())
  {
    const int key = tile->first + tile->second * iter.column;
    if (auto cacheTile = cache.tileCache.find(key); cacheTile)
    {
      tiles.push_back(*cacheTile);
    }
    else
    {
      const auto rect = SkRect::MakeXYWH(
        tile->first * iter.tileWidth + cache.globalBound.left(),
        tile->second * iter.tileHeight + cache.globalBound.top(),
        iter.tileWidth,
        iter.tileHeight);
      auto v = cache.tileCache.insert(key, { nullptr, rect });
      v->image = rasterTile(surface, rasterContext.picture, cache.rasterMatrix, rect);
      tiles.push_back(*v);
    }
  }
  if (rasterIter == iter)
  {
    return tiles;
  }
  while (auto rt = rasterIter.next())
  {
    const int key = rt->first + rt->second * iter.column;
    if (!iter.contains(rt->first, rt->second) && !cache.tileCache.find(key))
    {
      const auto rect = SkRect::MakeXYWH(
        rt->first * iter.tileWidth + cache.globalBound.left(),
        rt->second * iter.tileHeight + cache.globalBound.top(),
        iter.tileWidth,
        iter.tileHeight);
      auto v = cache.tileCache.insert(key, { nullptr, rect });
      v->image = rasterTile(surface, rasterContext.picture, cache.rasterMatrix, rect);
    }
  }
  return tiles;
}
} // namespace

namespace VGG::layer
{

class RasterCacheTile__pImpl
{
  VGG_DECL_API(RasterCacheTile);

public:
  std::array<RasterCacheTile::CacheState, Zoomer::ZOOM_LEVEL_COUNT + 1> cacheStack;
  const float                                                           tileWidth = 1024.f;
  const float                                                           tileHeight = 1024.f;
  sk_sp<SkSurface>                                                      surface;
  RasterCacheTile__pImpl(RasterCacheTile* api, float w, float h)
    : q_ptr(api)
    , tileWidth(w)
    , tileHeight(h)
  {
  }

  SkSurface* rasterSurface(GrRecordingContext* context)
  {
    if (!surface)
    {
      ASSERT(tileWidth > 0 && tileHeight > 0);
      auto info = SkImageInfo::MakeN32Premul(tileWidth, tileHeight);
      surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
      if (!surface)
      {
        return nullptr;
      }
    }
    return surface.get();
  }

  void invalidateContent()
  {
    for (auto& c : cacheStack)
    {
      c.invalid = true;
    }
  }

  std::string printReason(uint32_t r)
  {
    std::string res;
    if (r == 0)
      return "";
    if (r & Rasterizer::ZOOM_TRANSLATION)
    {
      res += " ZOOM_TRANSLATION";
    }
    if (r & Rasterizer::ZOOM_SCALE)
    {
      res += " ZOOM_SCALE";
    }
    if (r & Rasterizer::VIEWPORT)
    {
      res += " VIEWPORT";
    }
    if (r & Rasterizer::CONTENT)
    {
      res += " CONTENT";
    }
    return res;
  }

  void revalidate(
    RasterCacheTile::CacheState& levelCache,
    const SkMatrix&              totalMatrix,
    const SkRect&                bound)
  {
    if (levelCache.invalid)
    {
      levelCache.rasterMatrix = totalMatrix;
      levelCache.rasterMatrix[SkMatrix::kMTransX] = 0;
      levelCache.rasterMatrix[SkMatrix::kMTransY] = 0;
      levelCache.globalBound = levelCache.rasterMatrix.mapRect(bound);
      levelCache.tileCache.purge();
      levelCache.tileHeight = tileHeight;
      levelCache.tileWidth = tileWidth;
      levelCache.invalid = false;
    }
  }
};

std::tuple<uint32_t, std::vector<Rasterizer::Tile>, SkMatrix> RasterCacheTile::onRevalidateRaster(
  uint32_t             reason,
  GrRecordingContext*  context,
  int                  lod,
  const SkRect&        clipRect,
  const RasterContext& rasterContext,
  void*                userData)
{
  // DEBUG("reason: %s", printReason(reason).c_str());
  VGG_IMPL(RasterCacheTile);
  ASSERT(_->cacheStack.size() > 0);
  ASSERT(lod < int(_->cacheStack.size()) - 1);

  size_t cacheIndex = lod < 0 ? _->cacheStack.size() - 1 : lod;
  auto&  currentLevelCache = _->cacheStack[cacheIndex];

  auto       hitMatrix = SkMatrix::I();
  const auto totalMatrix = rasterContext.globalMatrix * rasterContext.localMatrix;
  hitMatrix[SkMatrix::kMTransX] = totalMatrix.getTranslateX();
  hitMatrix[SkMatrix::kMTransY] = totalMatrix.getTranslateY();
  const auto skv = clipRect.makeOffset(
    -totalMatrix.getTranslateX(),
    -totalMatrix.getTranslateY()); // clipRect * inv(hitMatrix)
  auto reval = [&](const SkRect& clipRect, const SkRect& rasterRect)
    -> std::tuple<uint32_t, std::vector<Rasterizer::Tile>, SkMatrix>
  {
    _->revalidate(currentLevelCache, totalMatrix, *rasterContext.bound);
    auto iter = TileIterator(skv, _->tileWidth, _->tileHeight, currentLevelCache.globalBound);
    auto rasterIter =
      TileIterator(rasterRect, _->tileWidth, _->tileHeight, currentLevelCache.globalBound);
    if (!iter.valid())
    {
      DEBUG("no tile hit");
      return { reason, {}, hitMatrix };
    }
    auto surface = _->rasterSurface(context);
    auto tiles = rasterOrGetTiles(surface, rasterContext, currentLevelCache, iter, rasterIter);
    return { reason, std::move(tiles), hitMatrix };
  };

  if (reason & CONTENT)
  {
    _->invalidateContent();
    DEBUG("content changed");
    const auto preCacheRect =
      clipRect.makeOutset(clipRect.width(), clipRect.height() * 3)
        .makeOffset(-totalMatrix.getTranslateX(), -totalMatrix.getTranslateY());
    return reval(skv, preCacheRect);
  }
  if ((reason & ZOOM_TRANSLATION) && !(reason & ZOOM_SCALE)) // most case
  {
    DEBUG("translation");
    return reval(skv, skv);
  }
  if (reason & ZOOM_SCALE)
  {
    DEBUG("scale changed");
    if (lod < 0)
    {
      currentLevelCache.invalid = true;
    }
    return reval(skv, skv);
  }
  if (reason & VIEWPORT)
  {
    DEBUG("viewport changed");
    return reval(skv, skv);
  }
  DEBUG("other changed");
  return reval(skv, skv);
}

void RasterCacheTile::purge()
{
  VGG_IMPL(RasterCacheTile);
  for (auto& c : _->cacheStack)
  {
    c.tileCache.purge();
    c.invalid = true;
  }
}

RasterCacheTile::RasterCacheTile(float tw, float th)
  : d_ptr(new RasterCacheTile__pImpl(this, tw, th))
{
}

RasterCacheTile::~RasterCacheTile()
{
}

} // namespace VGG::layer
