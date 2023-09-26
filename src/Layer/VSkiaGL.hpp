#pragma once

#include "VSkiaContext.hpp"

#include "Layer/Graphics/ContextInfoGL.hpp"

#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/GrBackendSurface.h>
#include <src/gpu/ganesh/gl/GrGLDefines.h>

namespace VGG::layer::skia_impl::gl
{
SkiaContext::SurfaceCreateProc glSurfaceCreateProc()
{
  return [](GrDirectContext* context, int w, int h, const ContextConfig& cfg)
  {
    GrGLFramebufferInfo info;
    info.fFBOID = 0;
    info.fFormat = GR_GL_RGBA8;
    GrBackendRenderTarget target(w, h, cfg.multiSample, cfg.stencilBit, info);
    SkSurfaceProps props;
    return SkSurfaces::WrapBackendRenderTarget(context,
                                               target,
                                               kBottomLeft_GrSurfaceOrigin,
                                               SkColorType::kRGBA_8888_SkColorType,
                                               nullptr,
                                               &props);
  };
}

SkiaContext::ContextCreateProc glContextCreateProc(ContextInfoGL* context)
{
  return [context]()
  {
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    ASSERT(interface);
    sk_sp<GrDirectContext> grContext = GrDirectContext::MakeGL(interface);
    ASSERT(grContext);
    return grContext;
  };
}

} // namespace VGG::layer::skia_impl::gl
