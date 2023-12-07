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
#include "Layer/Core/Timer.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"
#include "Layer/Graphics/GraphicsSkia.hpp"

#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkPictureRecorder.h>
#include <include/docs/SkPDFDocument.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkPicture.h>
#include <include/core/SkPictureRecorder.h>
#include <include/core/SkImage.h>
#include <include/core/SkSwizzle.h>
#include <include/core/SkTextBlob.h>
#include <include/core/SkTime.h>
#include <include/core/SkColorSpace.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/core/SkStream.h>
#include <encode/SkPngEncoder.h>

namespace VGG::layer::skia_impl
{

class SkiaContext
{
public:
  SkiaContext(const SkiaContext&) = delete;
  SkiaContext& operator=(const SkiaContext&) = delete;
  SkiaContext(SkiaContext&& other) noexcept
    : m_stream(std::move(other.m_stream))
    , m_document(std::move(other.m_document))
    , m_grContext(std::move(other.m_grContext))
    , m_surface(std::move(other.m_surface))
    , m_ctxConfig(std::move(other.m_ctxConfig))
    , m_skiaSurfaceCreateProc(std::move(other.m_skiaSurfaceCreateProc))
    , m_skiaContextCreateProc(std::move(other.m_skiaContextCreateProc))
  {
  }
  SkiaContext& operator=(SkiaContext&& other) noexcept
  {
    release();
    m_stream = std::move(other.m_stream);
    m_document = std::move(other.m_document);
    m_surface = std::move(other.m_surface);
    m_grContext = std::move(other.m_grContext);
    return *this;
  }

  void release()
  {
    DEBUG("SkiaContext Releasing ... ");
    // ensure release order
    m_stream = nullptr;
    m_document = nullptr;
    m_surface = nullptr;
    m_grContext = nullptr;
  }

private:
  using Size = std::array<int, 2>;

  std::unique_ptr<SkFILEWStream> m_stream;
  sk_sp<SkDocument>              m_document;
  sk_sp<GrDirectContext>         m_grContext{ nullptr };
  sk_sp<SkSurface>               m_surface{ nullptr };
  SurfaceCreateProc              m_skiaSurfaceCreateProc;
  ContextCreateProc              m_skiaContextCreateProc;
  ContextConfig                  m_ctxConfig;
  std::optional<Size>            m_surfaceSize;
  bool                           m_surfaceSwapped{ false };

  void clearSwap()
  {
    m_surfaceSwapped = false;
  }

public:
  SkiaContext(
    ContextCreateProc    contextProc,
    SurfaceCreateProc    surfaceProc,
    const ContextConfig& cfg)
    : m_skiaContextCreateProc(std::move(contextProc))
    , m_skiaSurfaceCreateProc(std::move(surfaceProc))
    , m_ctxConfig(cfg)
  {
    m_grContext = m_skiaContextCreateProc();
    ASSERT(m_grContext);
  }

  void resizeSurface(int w, int h)
  {
    ASSERT(m_skiaSurfaceCreateProc);
    if (m_surfaceSize)
    {
      auto [width, height] = *m_surfaceSize;
      if (width == w && height == h)
        return;
    }
    m_surface = m_skiaSurfaceCreateProc(m_grContext.get(), w, h, m_ctxConfig);
    m_surfaceSize = { w, h };
    if (m_surface)
      clearSwap();
    else
      DEBUG("In resizeSurface: Recreate surface failed");
  }

  bool flushAndSubmit()
  {
    auto ok = m_grContext->submit();
    m_grContext->flush();
    return ok;
  }

  void markSwap()
  {
    m_surfaceSwapped = true;
  }

  bool prepareFrame()
  {
    if (m_surfaceSize && m_surfaceSwapped)
    {
      auto [w, h] = *m_surfaceSize;
      m_surface = m_skiaSurfaceCreateProc(m_grContext.get(), w, h, m_ctxConfig);
      if (m_surface)
        clearSwap();
      else
        DEBUG("In prepareFrame: Recreate surface failed");
    }
    if (m_surface && !m_surfaceSwapped)
      return true;
    return false;
  }

  SkSurface* surface()
  {
    ASSERT(m_surface);
    return m_surface.get();
  }

  SkCanvas* canvas()
  {
    ASSERT(m_surface);
    return m_surface->getCanvas();
  }

  GrDirectContext* context()
  {
    return m_grContext.get();
  }

  ~SkiaContext()
  {
    release();
  }
};
}; // namespace VGG::layer::skia_impl
