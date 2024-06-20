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

#include <core/SkRect.h>
#include <optional>
#include <Layer/Core/VBounds.hpp>

namespace VGG::layer
{

struct TileIterator
{
  TileIterator(const SkRect& clip, int tileW, int tileH, const SkRect& bounds)
    : tileWidth(tileW)
    , tileHeight(tileH)
    , beginX(std::max(0, int((clip.x() - bounds.x()) / tileW)))
    , beginY(std::max(0, int((clip.y() - bounds.y()) / tileH)))
    , endX(
        std::min(std::ceil(bounds.width() / tileW), std::ceil((clip.right() - bounds.x()) / tileW)))
    , endY(std::min(
        std::ceil(bounds.height() / tileH),
        std::ceil((clip.bottom() - bounds.y()) / tileH)))
    , column(std::ceil(bounds.width() / tileW))
  {
    m_x = beginX;
    m_y = beginY;
  }

  TileIterator(TileIterator&&) = default;
  TileIterator(const TileIterator&) = default;
  TileIterator& operator=(TileIterator&&) = delete;
  TileIterator& operator=(const TileIterator&) = delete;

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

  int col() const
  {
    return column;
  }

  int row() const
  {
    return endY - beginY;
  }

  int index(int x, int y) const
  {
    return x + y * column;
  }

  std::pair<int, int> pos(int x, int y) const
  {
    return std::make_pair(x * tileWidth, y * tileHeight);
  }

  Boundsi bounds(int x, int y) const
  {
    return Boundsi{ x * tileWidth, y * tileHeight, tileWidth, tileHeight };
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

  const int tileWidth, tileHeight;
  const int beginX, beginY;
  const int endX, endY;
  const int column;

private:
  int m_x, m_y;
};
} // namespace VGG::layer
