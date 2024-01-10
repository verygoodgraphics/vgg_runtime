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
#include "Layer/Core/VBound.hpp"
#include <core/SkImage.h>
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

class RasterCache
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
  };
  enum EReason : uint32_t
  {
    ZOOM_TRANSLATION = 1,
    ZOOM_SCALE = 2,
    VIEWPORT = 4,
    CONTENT = 8,
    ALL = ZOOM_TRANSLATION | ZOOM_SCALE | VIEWPORT | CONTENT
  };
  virtual ~RasterCache() = default;
  RasterCache() = default;
  RasterCache(const RasterCache&) = delete;
  RasterCache& operator=(const RasterCache&) = delete;
  RasterCache(RasterCache&&) = delete;
  RasterCache& operator=(RasterCache&&) = delete;

  void invalidate(EReason reason)
  {
    m_reason |= reason;
  }
  bool isInvalidate() const
  {
    return m_reason != 0;
  }
  void raster(
    GrRecordingContext* context,
    const SkMatrix*     transform,

    SkPicture*       pic,
    const Bound&     bound,
    const glm::mat3& mat,

    const Zoomer* zoomer,
    const Bound&  viewport,
    void*         userData)
  {
    if (isInvalidate())
    {
      auto clear =
        onRaster(m_reason, context, transform, pic, bound, mat, zoomer, viewport, userData);
      m_reason &= ~clear;
    }
  }

  void queryTile(std::vector<Tile>* tiles, SkMatrix* transform)
  {
    onQueryTile(tiles, transform);
  }

protected:
  virtual uint32_t onRaster(
    uint32_t            reason,
    GrRecordingContext* context,
    const SkMatrix*     transform,
    SkPicture*          pic,
    const Bound&        bound,
    const glm::mat3&    mat,
    const Zoomer*       zoomer,
    const Bound&        viewport,
    void*               userData) = 0;

  virtual void onQueryTile(std::vector<Tile>* tiles, SkMatrix* transform) = 0;

private:
  uint32_t m_reason;
};
} // namespace VGG::layer
