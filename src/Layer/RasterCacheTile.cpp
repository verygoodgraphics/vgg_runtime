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

#include "TileIterator.hpp"
#include "Layer/Core/RasterCacheTile.hpp"
#include "Layer/Core/RasterCache.hpp"

#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/LRUCache.hpp"
#include "core/SkCanvas.h"
#include "core/SkImage.h"
#include "core/SkPicture.h"
#include "core/SkSurface.h"
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <optional>

namespace
{
using namespace VGG::layer;
struct CacheState
{
  using TileMap =
    LRUCache<int, std::pair<bool, Rasterizer::Tile>>; // boolean indicates if the tile is valid
  SkMatrix rasterMatrix;
  TileMap  tileCache;
  SkRect   rasterBounds;
  int      tileWidth;
  int      tileHeight;
  CacheState(int size = 40)
    : tileCache(size)
  {
  }

  void inval()
  {
    m_invalid = true;
  }

  void revalidate(const Rasterizer::RasterContext& rasterContext, const SkRect& clipRect)
  {
    if (!isInvalid())
      return;
    rasterMatrix = rasterContext.globalMatrix * rasterContext.localMatrix;
    rasterMatrix[SkMatrix::kMTransX] = 0;
    rasterMatrix[SkMatrix::kMTransY] = 0;
    rasterBounds = rasterMatrix.mapRect(*rasterContext.bounds);
    calcTileSize(rasterBounds.width(), rasterBounds.height(), clipRect);
    for (auto it = tileCache.begin(); it != tileCache.end(); it++)
    {
      ASSERT(*it);
      (*it)->value.first = false;
    }
    m_invalid = false;
  }

private:
  bool isInvalid() const
  {
    return m_invalid;
  }

  void calcTileSize(float w, float h, const SkRect& clipRect)
  {
    ASSERT(w > 0 && h > 0);
    (void)clipRect;
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
    tileWidth = newTileWidth, clipRect.width();
    tileHeight = newTileHeight;
    if (w / h > 3)
    {
      tileWidth = newTileHeight;
      tileHeight = newTileWidth;
    }
    tileWidth = w < clipRect.width() ? std::ceil(w) : tileWidth;
    tileHeight = h < clipRect.height() ? std::ceil(h) : tileHeight;
    ASSERT(tileHeight > 0 && tileWidth > 0);
  }

  bool m_invalid{ true };
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
  SkSurface*  surface,
  SkPicture*  picture,
  CacheState& cache,
  TileIter    iter,
  TileIter    rasterIter)
{
  std::vector<Rasterizer::Tile> tiles;
  tiles.reserve(8);
  auto makeRect = [](int tw, int th, int x, int y, float offsetX, float offsetY)
  { return SkRect::MakeXYWH(x * tw + offsetX, y * th + offsetY, tw, th); };

  while (auto tile = iter.next())
  {
    const int key = tile->index();
    if (auto tileState = cache.tileCache.find(key); tileState)
    {
      if (!tileState->first)
      {
        auto rect = makeRect(
          iter.tileWidth,
          iter.tileHeight,
          tile->c,
          tile->r,
          cache.rasterBounds.left(),
          cache.rasterBounds.top());
        tileState->first = true;
        tileState->second.image = rasterTile(surface, picture, cache.rasterMatrix, rect);
      }
      tiles.push_back(tileState->second);
    }
    else
    {
      const auto rect = makeRect(
        iter.tileWidth,
        iter.tileHeight,
        tile->c,
        tile->r,
        cache.rasterBounds.left(),
        cache.rasterBounds.top());
      auto v = cache.tileCache.insert(key, { true, { nullptr, rect } });
      v->second.image = rasterTile(surface, picture, cache.rasterMatrix, rect);
      tiles.push_back(v->second);
    }
  }
  if (rasterIter == iter)
  {
    return tiles;
  }
  while (auto rt = rasterIter.next())
  {
    const int key = rt->index();
    if (!iter.contains(rt->c, rt->r))
    {
      auto tileState = cache.tileCache.find(key);
      if (!tileState)
      {
        const auto rect = makeRect(
          iter.tileWidth,
          iter.tileHeight,
          rt->c,
          rt->r,
          cache.rasterBounds.left(),
          cache.rasterBounds.top());
        auto v = cache.tileCache.insert(key, { true, { nullptr, rect } });
        v->second.image = rasterTile(surface, picture, cache.rasterMatrix, rect);
      }
      else if (!tileState->first)
      {
        tileState->first = true;
        const auto rect = makeRect(
          iter.tileWidth,
          iter.tileHeight,
          rt->c,
          rt->r,
          cache.rasterBounds.left(),
          cache.rasterBounds.top());
        tileState->second.image = rasterTile(surface, picture, cache.rasterMatrix, rect);
      }
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
  std::array<CacheState, ZoomerNode::ZOOM_LEVEL_COUNT + 1> cacheStack;
  sk_sp<SkSurface>                                         surface;
  RasterCacheTile__pImpl(RasterCacheTile* api)
    : q_ptr(api)
  {
  }

  SkSurface* rasterSurface(GrRecordingContext* context, int w, int h)
  {
    ASSERT(w > 0 && h > 0);
    if (!surface || surface->width() != w || surface->height() != h)
    {
      auto info = SkImageInfo::MakeN32Premul(w, h);
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
      c.inval();
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

  const size_t cacheIndex = lod < 0 ? _->cacheStack.size() - 1 : lod;
  auto&        cache = _->cacheStack[cacheIndex];

  auto       hitMatrix = SkMatrix::I();
  const auto totalMatrix = rasterContext.globalMatrix * rasterContext.localMatrix;
  hitMatrix[SkMatrix::kMTransX] = totalMatrix.getTranslateX();
  hitMatrix[SkMatrix::kMTransY] = totalMatrix.getTranslateY();
  const auto skv = clipRect.makeOffset(
    -totalMatrix.getTranslateX(),
    -totalMatrix.getTranslateY()); // inv(hitMatrix) * clipRect
  auto reval = [&](const SkRect& clipRect, const SkRect& rasterRect)
    -> std::tuple<uint32_t, std::vector<Rasterizer::Tile>, SkMatrix>
  {
    cache.revalidate(rasterContext, clipRect);
    auto iter = TileIter(
      Bounds{ clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height() },
      cache.tileWidth,
      cache.tileHeight,
      Bounds{ cache.rasterBounds.x(),
              cache.rasterBounds.y(),
              cache.rasterBounds.width(),
              cache.rasterBounds.height() });
    auto rasterIter = TileIter(
      Bounds{ rasterRect.x(), rasterRect.y(), rasterRect.width(), rasterRect.height() },
      cache.tileWidth,
      cache.tileHeight,
      Bounds{ cache.rasterBounds.x(),
              cache.rasterBounds.y(),
              cache.rasterBounds.width(),
              cache.rasterBounds.height() });
    if (!iter.valid())
    {
      DEBUG("no tile hit");
      return { reason, {}, hitMatrix };
    }
    auto surface = _->rasterSurface(context, cache.tileWidth, cache.tileHeight);
    auto tiles = rasterOrGetTiles(surface, rasterContext.picture, cache, iter, rasterIter);
    return { reason, std::move(tiles), hitMatrix };
  };

  if (reason & CONTENT)
  {
    purge();
    _->invalidateContent();
    DEBUG("content changed");
    const auto preCacheRect =
      clipRect.makeOutset(clipRect.width(), clipRect.height() * 4)
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
      purge();
      _->invalidateContent();
      cache.inval();
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
  }
}

RasterCacheTile::RasterCacheTile()
  : d_ptr(new RasterCacheTile__pImpl(this))
{
}

RasterCacheTile::~RasterCacheTile()
{
}

} // namespace VGG::layer
