#pragma once
#include "VGG/Layer/Graphics/GraphicsContext.h"

#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/GrDirectContext.h>
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

template<class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

class SkiaContext
{
public:
  enum class ESkiaGraphicsAPIBackend
  {
    API_OPENGL,
    API_VULKAN
  };

  using SurfaceCreateProc = std::function<
    sk_sp<SkSurface>(GrDirectContext* grContext, int w, int h, const ContextConfig& cfg)>;
  using ContextCreateProc = std::function<sk_sp<GrDirectContext>()>;

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
    *this = SkiaContext(std::move(other));
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
  std::unique_ptr<SkFILEWStream> m_stream;
  sk_sp<SkDocument> m_document;
  sk_sp<GrDirectContext> m_grContext{ nullptr };
  sk_sp<SkSurface> m_surface{ nullptr };
  SurfaceCreateProc m_skiaSurfaceCreateProc;
  ContextCreateProc m_skiaContextCreateProc;
  ContextConfig m_ctxConfig;

public:
  SkiaContext(ContextCreateProc contextProc,
              SurfaceCreateProc surfaceProc,
              const ContextConfig& cfg)
    : m_skiaContextCreateProc(std::move(contextProc))
    , m_skiaSurfaceCreateProc(std::move(surfaceProc))
    , m_ctxConfig(cfg)
  {
    m_grContext = m_skiaContextCreateProc();
    ASSERT(m_grContext);
    // m_surface = m_skiaSurfaceCreateProc(m_grContext.get(),
    //                                     m_ctxConfig.windowSize[0],
    //                                     m_ctxConfig.windowSize[1],
    //                                     m_ctxConfig);
    // if (m_surface == nullptr)
    // {
    //   DEBUG("null m_surface");
    // }
    // ASSERT(m_surface);
  }

  void resizeSurface(int w, int h)
  {
    ASSERT(m_skiaSurfaceCreateProc);
    m_surface = m_skiaSurfaceCreateProc(m_grContext.get(), w, h, m_ctxConfig);
  }

  bool flushAndSubmit()
  {
    auto ok = m_grContext->submit();
    m_grContext->flush();
    return ok;
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
