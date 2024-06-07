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
#pragma once

#include "Layer/Core/VBounds.hpp"
#include "Layer/Core/VNode.hpp"
// #include <gpu/GrRecordingContext.h>

#include <core/SkImage.h>
#include <core/SkSurface.h>
#include <core/SkCanvas.h>

class GrRecordingContext;
namespace VGG::layer
{

class TileManager : public VNode
{
  int    m_width, m_height;
  Bounds m_rasterBounds;

public:
  TileManager(VRefCnt* cnt, int tileWidth, int tileHeight, const Bounds& bounds)
    : VNode(cnt)
    , m_width(tileWidth)
    , m_height(tileHeight)
  {
  }

  std::vector<int> query(const Bounds& bounds) const;
};

class Tile
{
  sk_sp<SkSurface> m_surf;
  int              m_index = -1;

public:
  Tile(sk_sp<SkSurface> surf, int index)
    : m_surf(std::move(surf))
    , m_index(index)
  {
  }

  int index() const
  {
    return m_index;
  }

  void blit(sk_sp<SkSurface> surf, int x, int y)
  {
    auto cvs = surf->getCanvas();
    cvs->drawImage(surf->makeImageSnapshot(), x, y);
  }

  sk_sp<SkImage> image() const
  {
    return m_surf->makeImageSnapshot();
  }
};

class TileProxy
{
  std::shared_ptr<Tile> m_target;
  mutable std::mutex    m_mut;
  int                   m_rasterX, m_rasterY;
  TileManager*          m_mgr;

  bool m_dirty{ false };

public:
  TileProxy(TileManager* mgr, std::shared_ptr<Tile> target, int rx, int ry)
    : m_target(target)
    , m_rasterX(rx)
    , m_rasterY(ry)
    , m_mgr(mgr)
  {
    // ASSERT(m_mgr);
  }

  void blit(sk_sp<SkSurface> surf, int rx, int ry);

  TileManager* owner() const
  {
    return m_mgr;
  }

  void update(sk_sp<SkPicture> picture, Boundsi bounds)
  {
  }
};

class RasterCache
{
public:
  RasterCache();
  RasterCache(const RasterCache&) = delete;
  RasterCache& operator=(const RasterCache&) = delete;
  RasterCache(RasterCache&&) = delete;
  RasterCache& operator=(RasterCache&&) = delete;

  std::vector<Bounds> query(const Bounds& bounds) const;

private:
};
} // namespace VGG::layer
