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

  struct Key
  {
    GrRecordingContext* context;
    const SkMatrix*     transform;
    SkPicture*          pic;
    const SkRect&       bound;
    const SkMatrix&     mat;
    const SkRect&       viewport;
    void*               userData;

    Key(
      GrRecordingContext* context,
      const SkMatrix*     transform,
      const SkRect&       clipRect,
      SkPicture*          pic,
      const SkRect&       bound,
      const SkMatrix&     mat,
      void*               userData)
      : context(context)
      , transform(transform)
      , pic(pic)
      , bound(bound)
      , mat(mat)
      , viewport(clipRect)
      , userData(userData)
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

  void rasterize(const Key& key, Tile** tiles, int* count, SkMatrix* transform)
  {
    if (isInvalidate())
    {
      auto clear = onRaster(
        m_reason,
        key.context,
        key.transform,
        key.viewport,
        key.pic,
        key.bound,
        key.mat,
        key.userData);
      m_reason &= ~clear;
    }
    onQueryTile(tiles, count, transform);
  }

protected:
  virtual uint32_t onRaster(
    uint32_t            reason,
    GrRecordingContext* context,
    const SkMatrix*     transform,
    const SkRect&       clipRect,
    SkPicture*          pic,
    const SkRect&       bound,
    const SkMatrix&     mat,
    void*               userData) = 0;

  virtual void onQueryTile(Tile** tiles, int* count, SkMatrix* transform) = 0;

private:
  uint32_t m_reason;
};
} // namespace VGG::layer
