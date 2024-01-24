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
#include <algorithm>
#include <core/SkMatrix.h>
#include <core/SkSurface.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <gpu/GpuTypes.h>

class Zoomer;
namespace VGG::layer
{
class RasterCacheTile : public Rasterizer
{

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

  // using TileMap = std::unordered_map<SkRect, RasterCache::Tile, SkRectHash>;
  using TileMap = std::vector<Rasterizer::Tile>;
  std::vector<TileMap::iterator> hitTile(const SkRect& viewport, const SkMatrix& transform);
  void                           calculateTiles(const SkRect& content);
  void revalidate(const SkMatrix& transform, const SkMatrix& localMatrix, const SkRect& bound);
  const SkMatrix& rasterMatrix() const
  {
    return m_rasterMatrix;
  }

  const SkMatrix& hitMatrix() const
  {
    return m_hitMatrix;
  }

public:
  RasterCacheTile() = default;
  RasterCacheTile(float tw, float th)
    : m_tileWidth(tw)
    , m_tileHeight(th)
  {
  }

  void onTiles(std::vector<Tile>* tiles, SkMatrix* transform) override
  {
    *tiles = m_hitTiles;
    *transform = m_hitMatrix;
  }

  uint32_t onRevalidateRaster(
    uint32_t             reason,
    GrRecordingContext*  context,
    const SkRect&        clipRect,
    const RasterContext& rasterContext,
    void*                userData) override;

private:
  TileMap           m_tileCache;
  std::vector<Tile> m_hitTiles;
  SkMatrix          m_rasterMatrix;
  SkMatrix          m_hitMatrix{ SkMatrix::I() };
  const float       m_tileWidth = 1024.f;
  const float       m_tileHeight = 1024.f;
};

} // namespace VGG::layer
