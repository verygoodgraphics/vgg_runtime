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
#pragma once
#include "Layer/Core/RasterCache.hpp"
#include "Layer/VSkia.hpp"
#include "Layer/Zoomer.hpp"
#include "Utility/Log.hpp"

#include "core/SkCanvas.h"
#include "core/SkImage.h"
#include "core/SkPicture.h"
#include <core/SkSurface.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <gpu/GpuTypes.h>

class Zoomer;
namespace VGG::layer
{
class RasterCacheTile : public RasterCache
{
  SkMatrix m_transform;

  std::string printReason(uint32_t r)
  {
    std::string res;
    if (r == 0)
      return "";
    if (r & ZOOM_TRANSLATION)
    {
      res += " ZOOM_TRANSLATION";
    }
    if (r & ZOOM_SCALE)
    {
      res += " ZOOM_SCALE";
    }
    if (r & VIEWPORT)
    {
      res += " VIEWPORT";
    }
    if (r & CONTENT)
    {
      res += " CONTENT";
    }
    return res;
  }

  struct SkRectLess
  {
    bool operator()(const SkRect& lhs, const SkRect& rhs) const
    {
      return lhs.x() < rhs.x() && lhs.y() < rhs.y() && lhs.width() < rhs.width();
    }
  };

  struct SkRectHash
  {
    std::size_t operator()(const SkRect& k) const
    {
      using std::hash;
      using std::size_t;
      using std::string;

      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:

      return ((hash<float>()(k.x()) ^ (hash<float>()(k.y()) << 1)) >> 1) ^
             (hash<float>()(k.width()) << 1);
    }
  };
  // using TileMap = std::unordered_map<SkRect, RasterCache::Tile, SkRectHash>;
  using TileMap = std::vector<std::tuple<SkRect, RasterCache::Tile>>;

  TileMap           m_tileCache;
  std::vector<Tile> m_hitTiles;

  std::vector<TileMap::iterator> hitTile(const SkRect& viewport, const SkMatrix& transform)
  {
    if (m_tileCache.empty())
      return {};
    std::vector<TileMap::iterator> res;
    res.reserve(4);
    DEBUG("Hit viewport: %f %f", viewport.width(), viewport.height());
    for (auto it = m_tileCache.begin(); it != m_tileCache.end(); ++it)
    {
      auto& [_, tile] = *it;
      auto r = transform.mapRect(tile.rect);
      if (r.intersects(viewport))
      {
        DEBUG("Hit tile: %f %f [%f %f] ", r.top(), r.left(), tile.rect.top(), tile.rect.left());
        res.push_back(it);
      }
    }
    return res;
  }

  void calculateTiles(const SkRect& content)
  {
    m_tileCache.clear();
    auto x = content.x();
    auto y = content.y();
    auto localX = 0.f;
    auto localY = 0.f;
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
        m_tileCache.push_back(
          { SkRect::MakeXYWH(localX, localY, tileW, tileH), { nullptr, tile } });
        tileY += tileH;
        localY += tileH;
      }
      tileY = y;
      localY = 0.f;

      tileX += tileW;
      localX += tileW;
    }
  }

  const float m_tileWidth = 1024.f;
  const float m_tileHeight = 1024.f;

public:
  RasterCacheTile() = default;
  RasterCacheTile(float tw, float th)
    : m_tileWidth(tw)
    , m_tileHeight(th)
  {
  }

  void onQueryTile(Tile** tiles, int* count, SkMatrix* transform) override
  {
    *tiles = m_hitTiles.data();
    *count = m_hitTiles.size();
    *transform = m_transform;
    // (*transform)[SkMatrix::kMScaleX] = 1;
    // (*transform)[SkMatrix::kMScaleY] = 1;
  }

  uint32_t onRaster(
    uint32_t            reason,
    GrRecordingContext* context,
    const SkMatrix*     transform,
    SkPicture*          pic,
    const Bound&        bound,
    const glm::mat3&    mat,
    const Zoomer*       zoomer,
    const Bound&        viewport,
    void*               userData) override
  {
    auto str = printReason(reason);
    DEBUG("recache reason: %s", str.c_str());

    DEBUG(
      "Viewport: %f %f %f %f",
      viewport.topLeft().x,
      viewport.topLeft().y,
      viewport.width(),
      viewport.height());

    auto mm = toSkMatrix(mat);
    auto skm = *transform * mm;
    auto skv = toSkRect(viewport);
    auto contentRect = toSkRect(bound);

    auto hitMatrix = skm;
    if (m_tileCache.empty())
    {
      calculateTiles(contentRect);
    }

    std::vector<TileMap::iterator> hitTiles;

    if ((reason & ZOOM_TRANSLATION) && !(reason & ZOOM_SCALE)) // most case
    {
      DEBUG("ignore zoom translation");
      hitTiles = hitTile(skv, hitMatrix);
      if (hitTiles.empty())
      {
        m_hitTiles.clear();
        return reason;
      }
    }

    if (reason & ZOOM_SCALE || reason & CONTENT)
    {
      m_tileCache.clear();
      calculateTiles(contentRect);
      hitTiles = hitTile(skv, hitMatrix);
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
      auto& [_, tile] = *t;
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
          m_hitTiles.clear();
        }
      }
      auto canvas = surface->getCanvas();
      canvas->clear(SK_ColorTRANSPARENT);
      canvas->save();
      auto tr = skm.mapRect(tile.rect);
      canvas->concat(skm);
      canvas->translate(-tr.x(), -tr.y());
      canvas->clipRect(tile.rect);
      canvas->drawPicture(pic);
      canvas->restore();
      tile.image = surface->makeImageSnapshot();
    }
    m_hitTiles.clear();
    for (auto& t : hitTiles)
    {
      m_hitTiles.push_back(std::get<1>(*t));
    }
    m_transform = skm;
    return reason;
  }
};

} // namespace VGG::layer
