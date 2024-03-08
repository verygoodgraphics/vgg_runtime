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
#include "Layer/Renderer.hpp"

#include <core/SkPictureRecorder.h>
#include <src/shaders/SkPictureShader.h>

namespace VGG::layer
{

class DisplayList
{
public:
  SkPicture* picture() const
  {
    return m_displayList ? m_displayList->picture().get() : nullptr;
  }
  void playback(Renderer* renderer)
  {
    renderer->canvas()->drawPicture(m_displayList->picture());
  }

  sk_sp<SkShader> asShader() const
  {
    return m_displayList;
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
  friend class DisplayListRecorder;
  DisplayList(sk_sp<SkPictureShader> shader)
    : m_displayList(std::move(shader))
  {
    ASSERT(m_displayList);
  }
  sk_sp<SkPictureShader> m_displayList;
  sk_sp<SkImageFilter>   m_imageFilter;
};

class DisplayListRecorder
{
public:
  DisplayListRecorder() = default;
  DisplayListRecorder(const DisplayListRecorder&) = delete;
  DisplayListRecorder& operator=(const DisplayListRecorder&) = delete;
  Renderer*            beginRecording(const SkRect& b, const SkMatrix& matrix)
  {
    m_bound = b;
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
  DisplayList finishRecording()
  {
    auto maskShader = sk_make_sp<SkPictureShader>(
      m_rec->finishRecordingAsPicture(),
      SkTileMode::kClamp,
      SkTileMode::kClamp,
      SkFilterMode::kNearest,
      &m_bound);
    ASSERT(maskShader);
    return DisplayList(maskShader);
  }

private:
  SkMatrix                           m_matrix;
  SkRect                             m_bound;
  std::unique_ptr<Renderer>          m_renderer;
  std::unique_ptr<SkPictureRecorder> m_rec;
};
} // namespace VGG::layer
