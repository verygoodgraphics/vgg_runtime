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

struct TileIter
{
  TileIter(const Bounds& clip, int tileW, int tileH, const Bounds& bounds)
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

  TileIter(TileIter&&) = default;
  TileIter(const TileIter&) = default;
  TileIter& operator=(TileIter&&) = delete;
  TileIter& operator=(const TileIter&) = delete;

  TileIter()
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

  struct Result
  {
    const TileIter& iterator;
    const int       r;
    const int       c;
    Result(TileIter& it, int c, int r)
      : iterator(it)
      , r(r)
      , c(c)
    {
    }

    int index() const
    {
      return r * iterator.column + c;
    }

    size_t key() const
    {
      static_assert(sizeof(size_t) == 8, "size_t must be 8 bytes");
      return (((size_t)iterator.tileWidth & 0xffff) << 48) +
             (((size_t)iterator.tileHeight & 0xffff) << 32) + (size_t)index();
      // return index();
    }

    glm::ivec2 topLeft() const
    {
      return { c * iterator.tileWidth, r * iterator.tileHeight };
    }

    Boundsi bounds() const
    {
      return Boundsi{ c * iterator.tileWidth,
                      r * iterator.tileHeight,
                      iterator.tileWidth,
                      iterator.tileHeight };
    }
  };

  std::optional<Result> next()
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
    auto r = Result(*this, m_x, m_y);
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

  bool contains(int x, int y) const
  {
    return x >= beginX && x < endX && y >= beginY && y < endY;
  }

  bool operator==(const TileIter& other) const
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
