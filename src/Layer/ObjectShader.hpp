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
#include "Layer/Renderer.hpp"

#include <core/SkPictureRecorder.h>
#include <src/shaders/SkPictureShader.h>

namespace VGG::layer
{

class ObjectShader
{
public:
  void render(Renderer* renderer)
  {
    SkPaint p;
    p.setAntiAlias(true);
    renderer->canvas()->drawPicture(m_picture);
  }

  sk_sp<SkShader> asShader() const
  {
    return m_displayList;
  }

  const SkRect& bounds() const
  {
    return m_bounds;
  }

  sk_sp<SkShader> asAlphaShader()
  {
    return SkShaders::Blend(SkBlendMode::kSrcIn, m_displayList, SkShaders::Color(0xFFFFFFFF));
  }

  sk_sp<SkImageFilter> asAlphaImageFilter()
  {
    return SkImageFilters::Shader(asAlphaShader());
  }

  sk_sp<SkImageFilter> asImageFilter()
  {
    if (!m_imageFilter)
    {
      m_imageFilter = SkImageFilters::Shader(m_displayList);
    }
    return m_imageFilter;
  }

private:
  friend class ObjectRecorder;
  ObjectShader(sk_sp<SkShader> shader, sk_sp<SkPicture> picture, const SkRect& bounds)
    : m_picture(picture)
    , m_displayList(std::move(shader))
    , m_bounds(bounds)
  {
    ASSERT(m_displayList);
  }
  sk_sp<SkPicture>     m_picture;
  sk_sp<SkShader>      m_displayList;
  sk_sp<SkImageFilter> m_imageFilter;
  SkRect               m_bounds;
};

class ObjectRecorder
{
public:
  ObjectRecorder() = default;
  ObjectRecorder(const ObjectRecorder&) = delete;
  ObjectRecorder& operator=(const ObjectRecorder&) = delete;
  Renderer*       beginRecording(const SkRect& b, const SkMatrix& matrix)
  {
    m_bounds = b;
    m_matrix = matrix;
    if (!m_rec)
    {
      m_rec = std::make_unique<SkPictureRecorder>();
      auto rt = SkRTreeFactory();
      auto canvas = m_rec->beginRecording(b, &rt);
      if (!m_renderer)
      {
        m_renderer = std::make_unique<Renderer>();
      }
      m_renderer->setCanvas(canvas);
    }
    return renderer();
  }

  Renderer* renderer() const
  {
    return m_renderer.get();
  }
  ObjectShader finishRecording(const SkRect& cropRect, const SkMatrix* matrix)
  {
    auto pic = m_rec->finishRecordingAsPictureWithCull(cropRect);
    auto maskShader = SkPictureShader::Make(
      pic,
      SkTileMode::kClamp,
      SkTileMode::kClamp,
      SkFilterMode::kNearest,
      matrix,
      &cropRect);
    ASSERT(maskShader);
    return ObjectShader(maskShader, pic, cropRect);
  }

  sk_sp<SkPicture> finishAsPicture(const SkRect& cropRect)
  {
    return m_rec->finishRecordingAsPictureWithCull(cropRect);
  }

private:
  SkMatrix                           m_matrix;
  SkRect                             m_bounds;
  std::unique_ptr<Renderer>          m_renderer;
  std::unique_ptr<SkPictureRecorder> m_rec;
};
} // namespace VGG::layer
