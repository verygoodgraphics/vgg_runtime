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
#include <core/SkImage.h>
#include <gpu/GrDirectContext.h>
#include <tuple>
#include <vector>

class GrRecordingContext;
class SkImage;
class SkPicture;
class SkCanvas;
class SkMatrix;
class Zoomer;

template<typename T>
class sk_sp;

namespace VGG::layer
{

class DisplayItem
{
public:
  virtual const SkRect&   clipRect() = 0;
  virtual SkPicture*      picture() = 0;
  virtual const SkMatrix& matrix() = 0;
};

class Rasterizer
{
public:
  struct Tile
  {
    sk_sp<SkImage> image;
    SkRect         rect;
    Tile(sk_sp<SkImage> image, SkRect rect)
      : image(std::move(image))
      , rect(rect)
    {
    }

    Tile()
      : image(nullptr)
      , rect(SkRect::MakeEmpty())
    {
    }

    Tile(Tile&& t) = default;
    Tile& operator=(Tile&& t) = default;
    Tile(const Tile& t) = default;
    Tile& operator=(const Tile& t) = default;
  };

  struct RasterContext
  {
    const SkMatrix& globalMatrix;
    SkPicture*      picture;
    const SkRect*   bounds;
    const SkMatrix& localMatrix;
    RasterContext(
      const SkMatrix& transform,
      SkPicture*      pic,
      const SkRect*   bounds,
      const SkMatrix& mat)
      : globalMatrix(transform)
      , picture(pic)
      , bounds(bounds)
      , localMatrix(mat)
    {
    }
  };

  enum EReason : uint32_t
  {
    ZOOM_TRANSLATION = 1,
    ZOOM_SCALE = 2,
    VIEWPORT = 4,
    CONTENT = 8,
    ALL = ZOOM_TRANSLATION | ZOOM_SCALE | VIEWPORT | CONTENT
  };

  virtual ~Rasterizer() = default;
  Rasterizer() = default;
  Rasterizer(const Rasterizer&) = delete;
  Rasterizer& operator=(const Rasterizer&) = delete;
  Rasterizer(Rasterizer&&) = delete;
  Rasterizer& operator=(Rasterizer&&) = delete;

  void invalidate(EReason reason)
  {
    m_reason |= reason;
  }

  bool isInvalidate() const
  {
    return m_reason != 0;
  }

  void rasterize(
    GrRecordingContext*  rasterDevice,
    const RasterContext& rasterContext,
    int                  lod,
    const SkRect&        clipRect,
    std::vector<Tile>*   tiles,
    SkMatrix*            transform,
    void*                userData = 0)
  {
    uint32_t clear;
    std::tie(clear, *tiles, *transform) =
      onRevalidateRaster(m_reason, rasterDevice, lod, clipRect, rasterContext, userData);
    m_reason &= ~clear;
  }

  virtual void purge() = 0;

protected:
  virtual std::tuple<uint32_t, std::vector<Tile>, SkMatrix> onRevalidateRaster(
    uint32_t             reason,
    GrRecordingContext*  context,
    int                  lod,
    const SkRect&        clipRect,
    const RasterContext& rasterContext,
    void*                userData) = 0;

private:
  uint32_t m_reason{ ALL };
};
} // namespace VGG::layer
