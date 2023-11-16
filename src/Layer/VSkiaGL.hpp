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

#include "VSkiaContext.hpp"
#include "Layer/Graphics/GraphicsSkia.hpp"
#include "Layer/Graphics/ContextInfoGL.hpp"

#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/GrBackendSurface.h>
#include <src/gpu/ganesh/gl/GrGLDefines.h>

namespace VGG::layer::skia_impl::gl
{
SurfaceCreateProc glSurfaceCreateProc()
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

ContextCreateProc glContextCreateProc(ContextInfoGL* context)
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
