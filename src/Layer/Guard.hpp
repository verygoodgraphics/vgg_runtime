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
#include <optional>
#include <core/SkCanvas.h>

#include "VSkia.hpp"

namespace VGG::layer
{

class SaveLayerContextGuard
{
  SkCanvas*              m_canvas;
  std::optional<SkPaint> m_paint;
  int                    m_saveCount = 0;

public:
  template<typename F = void(SkCanvas*, const SkPaint&)>
  SaveLayerContextGuard(SkCanvas* canvas, const ContextSetting& st, F&& f)
    : m_canvas(canvas)
    , m_paint(std::nullopt)
  {
    auto bm = toSkBlendMode(st.blendMode);
    m_saveCount = canvas->getSaveCount();
    if (bm)
    {
      std::visit(
        Overloaded{ [&, this](const sk_sp<SkBlender>& blender)
                    {
                      m_paint = SkPaint();
                      m_paint->setBlender(blender);
                    },
                    [&, this](const SkBlendMode& mode)
                    {
                      m_paint = SkPaint();
                      m_paint->setBlendMode(mode);
                    } },
        *bm);
    }
    if (st.opacity < 1.0)
    {
      if (!m_paint)
        m_paint = SkPaint();
      m_paint->setAlphaf(st.opacity);
    }
    if (m_paint)
    {
      f(canvas, *m_paint);
    }
  }

  void restore()
  {
    if (m_paint && m_canvas)
    {
      m_canvas->restoreToCount(m_saveCount);
      m_canvas = nullptr;
      m_paint = std::nullopt;
    }
  }

  ~SaveLayerContextGuard()
  {
    if (m_paint && m_canvas)
    {
      m_canvas->restoreToCount(m_saveCount);
    }
  }
  SaveLayerContextGuard(SaveLayerContextGuard&&) = delete;
  SaveLayerContextGuard(const SaveLayerContextGuard&) = delete;
  SaveLayerContextGuard& operator=(SaveLayerContextGuard&&) = delete;
  SaveLayerContextGuard& operator=(const SaveLayerContextGuard&) = delete;
};

class SaveLayerGuard
{
public:
  template<typename F = void(SkCanvas*)>
  SaveLayerGuard(SkCanvas* canvas, F&& f)
    : m_canvas(canvas)
    , m_saveCount(0)
  {
    if (m_canvas)
    {
      m_saveCount = canvas->getSaveCount();
      if (canvas)
      {
        f(canvas);
      }
    }
  }
  ~SaveLayerGuard()
  {
    if (m_canvas)
    {
      m_canvas->restoreToCount(m_saveCount);
    }
  }

  void restore()
  {
    if (m_canvas)
    {
      m_canvas->restoreToCount(m_saveCount);
      m_canvas = nullptr;
    }
  }

private:
  SkCanvas* m_canvas;
  int       m_saveCount;

  SaveLayerGuard(SaveLayerGuard&&) = delete;
  SaveLayerGuard(const SaveLayerGuard&) = delete;
  SaveLayerGuard& operator=(SaveLayerGuard&&) = delete;
  SaveLayerGuard& operator=(const SaveLayerGuard&) = delete;
};

} // namespace VGG::layer
