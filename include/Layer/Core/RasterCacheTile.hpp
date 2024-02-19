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

#include <core/SkMatrix.h>

class SkSurface;

class Zoomer;
namespace VGG::layer
{
class RasterCacheTile : public Rasterizer
{
public:
  using TileMap = LRUCache<int, Rasterizer::Tile>;
  struct LevelCache
  {
    SkMatrix rasterMatrix;
    TileMap  tileCache;
    SkRect   globalBound;
    int      tileWidth;
    int      tileHeight;
    bool     invalid{ true };
    LevelCache()
      : tileCache(20)
    {
    }
  };
  RasterCacheTile()
    : RasterCacheTile(1024, 1024)
  {
  }
  RasterCacheTile(float tw, float th);

  void purge() override
  {
    for (auto& c : m_cacheStack)
    {
      c.tileCache.purge();
      c.invalid = true;
    }
  }
  ~RasterCacheTile();

protected:
  std::tuple<uint32_t, std::vector<Tile>, SkMatrix> onRevalidateRaster(
    uint32_t             reason,
    GrRecordingContext*  context,
    int                  lod,
    const SkRect&        clipRect,
    const RasterContext& rasterContext,
    void*                userData) override;

private:
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

  void revalidate(
    LevelCache&     levelCache,
    const SkMatrix& totalMatrix,
    int             tileW,
    int             tileH,
    const SkRect&   bound);
  SkSurface* rasterSurface(GrRecordingContext* context);
  void       invalidateContent()
  {
    for (auto& c : m_cacheStack)
    {
      c.invalid = true;
    }
  }
  void hit(LevelCache& levelCache, const SkRect& clipBound, const SkRect& bound);

  std::array<LevelCache, Zoomer::ZOOM_LEVEL_COUNT + 1> m_cacheStack;
  const float                                          m_tileWidth = 1024.f;
  const float                                          m_tileHeight = 1024.f;
  sk_sp<SkSurface>                                     m_surface;
};

} // namespace VGG::layer
