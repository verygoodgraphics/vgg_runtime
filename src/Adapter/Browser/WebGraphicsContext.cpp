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

#include "WebGraphicsContext.hpp"

#include "Utility/Log.hpp"

// skia
#include <include/core/SkColorSpace.h>
#include <include/core/SkImageInfo.h>
#include <include/core/SkSurface.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <include/ports/SkCFObject.h>

#include <GLES3/gl3.h>
#include <emscripten/html5_webgl.h>

namespace VGG
{
// impl ------------------------------------------------------------------------
class WebGraphicsContextImpl
{
  WebGraphicsContext*    m_api;
  float                  m_devicePixelRatio;
  sk_sp<GrDirectContext> m_context;

  const EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_glContext;

public:
  WebGraphicsContextImpl(WebGraphicsContext* api)
    : m_api(api)
    , m_glContext(emscripten_webgl_get_current_context())
  {
    m_context = GrDirectContext::MakeGL();
  }

  bool swap()
  {
    return true;
  }

  SurfaceCreateProc surfaceCreateProc()
  {
    return [this](GrDirectContext* context, int w, int h, const VGG::layer::ContextConfig& cfg)
    {
      ASSERT(emscripten_webgl_get_current_context() == m_glContext);

      int numSamples, numStencilBits;
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glGetIntegerv(GL_SAMPLES, &numSamples);
      glGetIntegerv(GL_STENCIL_BITS, &numStencilBits);
      context->resetContext(kRenderTarget_GrGLBackendState);

      GrGLFramebufferInfo framebufferInfo;
      framebufferInfo.fFBOID = 0;
      framebufferInfo.fFormat = GL_RGBA8;

      GrBackendRenderTarget backendRenderTarget(w, h, numSamples, numStencilBits, framebufferInfo);

      return SkSurfaces::WrapBackendRenderTarget(
        context,
        backendRenderTarget,
        kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        nullptr,
        nullptr);
    };
  }

  ContextCreateProc contextCreateProc()
  {
    return [tmpContext = m_context]() { return tmpContext; };
  }

  void onInitProperties(VGG::layer::ContextProperty& property)
  {
    property.dpiScaling = m_devicePixelRatio;
    property.api = VGG::layer::EGraphicsAPIBackend::API_CUSTOM;
  }

  void init(float devicePixelRatio)
  {
    m_devicePixelRatio = devicePixelRatio;
  }

  bool makeCurrent()
  {
    emscripten_webgl_make_context_current(m_glContext);
    return true;
  }

private:
};

// api ------------------------------------------------------------------------
WebGraphicsContext::WebGraphicsContext()
  : m_impl(new WebGraphicsContextImpl(this))
{
}

void WebGraphicsContext::init(float devicePixelRatio)
{
  m_impl->init(devicePixelRatio);
}

bool WebGraphicsContext::swap()
{
  return m_impl->swap();
}

bool WebGraphicsContext::makeCurrent()
{
  return m_impl->makeCurrent();
}

void WebGraphicsContext::shutdown()
{
  return;
}

void* WebGraphicsContext::contextInfo()
{
  return nullptr;
}

bool WebGraphicsContext::onInit()
{
  return true;
}

SurfaceCreateProc WebGraphicsContext::surfaceCreateProc()
{
  return m_impl->surfaceCreateProc();
}

ContextCreateProc WebGraphicsContext::contextCreateProc()
{
  return m_impl->contextCreateProc();
}

void WebGraphicsContext::onInitProperties(VGG::layer::ContextProperty& property)
{
  m_impl->onInitProperties(property);
}

} // namespace VGG
