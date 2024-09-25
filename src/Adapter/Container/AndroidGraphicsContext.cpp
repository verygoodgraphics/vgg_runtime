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

#include "AndroidGraphicsContext.h"

// skia
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkGraphics.h"
#include "include/gpu/GrDirectContext.h"
#include "include/android/SkSurfaceAndroid.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"

// android
#include <android/native_window.h>
#include <android/native_window_jni.h>

// egl
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include "Utility/Log.hpp"

#include <cstring>
#include <memory>

namespace VGG
{
// impl ------------------------------------------------------------------------
class AndroidGraphicsContextImpl
{

  // RGBA_8888
  static constexpr uint32_t kBytePerPixel = 4;

  AndroidGraphicsContext* m_api = nullptr;
  ANativeWindow*          m_window = nullptr;
  // int                     m_currentBufferIndex = -1;
  sk_sp<GrDirectContext>  m_context;
  EGLDisplay              m_egl_display = nullptr;
  EGLSurface              m_egl_surface = nullptr;
  EGLContext              m_egl_context = nullptr;

public:
  AndroidGraphicsContextImpl(AndroidGraphicsContext* api, ANativeWindow* window)
    : m_api(api)
  {
    ASSERT(window);
    setView(window);
  }

  int width()
  {
    return m_window ? ANativeWindow_getWidth(m_window) : 0;
  }

  int height()
  {
    return m_window ? ANativeWindow_getHeight(m_window) : 0;
  }

  bool swap()
  {
    bool rc = false;
    if (m_window != nullptr && m_egl_display != nullptr && m_egl_surface != nullptr)
    {
      m_context->flush();
      rc = eglSwapBuffers(m_egl_display, m_egl_surface);
    }
    __android_log_print(ANDROID_LOG_DEBUG, "VGG", "swap %d", rc);
    return true;
  }

  SurfaceCreateProc surfaceCreateProc()
  {
    auto fn = [](GrRecordingContext* context, int w, int h, const VGG::layer::ContextConfig& cfg)
    {

      GrGLint buffer;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);

      GrGLFramebufferInfo bufInfo = {};
      bufInfo.fFormat = GL_RGBA8;
      bufInfo.fFBOID = buffer;

      GrBackendRenderTarget desc(w, h, 0, 8, bufInfo);

      auto surface = ::SkSurfaces::WrapBackendRenderTarget(
        context,
        desc,
        kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        nullptr,
        nullptr,
        nullptr);

      ASSERT(surface);

      return surface;
    };
    return fn;
  }

  ContextCreateProc contextCreateProc()
  {
    return [this]()
    {
      initContext();
      return m_context;
    };
  }

  void onInitProperties(VGG::layer::ContextProperty& property)
  {
    // TODO: implement dpi scaling
    property.dpiScaling = 1.f;
    property.api = VGG::layer::EGraphicsAPIBackend::API_CUSTOM;
  }

  void initContext()
  {
    if (m_context)
    {
      return;
    }

    ASSERT(m_window);

    // get current display, context and surface
    EGLDisplay display = eglGetCurrentDisplay();
    EGLContext context = eglGetCurrentContext();
    EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);
    ASSERT(display && context && surface);
    m_egl_display = display;
    m_egl_context = context;
    m_egl_surface = surface;

    auto grGlInterface = GrGLMakeNativeInterface();
    ASSERT(grGlInterface);
    m_context = GrDirectContext::MakeGL(grGlInterface);
    ASSERT(m_context);
  }

private:
  void setView(ANativeWindow* window)
  {
    m_window = window;
    if (!m_window)
    {
      // clear the context
      m_context.reset();
      m_egl_display = nullptr;
      m_egl_surface = nullptr;
      m_egl_context = nullptr;
    }
  }
};

// api ------------------------------------------------------------------------
AndroidGraphicsContext::AndroidGraphicsContext(ANativeWindow* window)
  : m_impl(new AndroidGraphicsContextImpl(this, window))
{
}

int AndroidGraphicsContext::width()
{
  return m_impl->width();
}

int AndroidGraphicsContext::height()
{
  return m_impl->height();
}

bool AndroidGraphicsContext::swap()
{
  return m_impl->swap();
}

bool AndroidGraphicsContext::makeCurrent()
{
  return true;
}

void AndroidGraphicsContext::shutdown()
{
  return;
}

void* AndroidGraphicsContext::contextInfo()
{
  return nullptr;
}

bool AndroidGraphicsContext::onInit()
{
  return true;
}

SurfaceCreateProc AndroidGraphicsContext::surfaceCreateProc()
{
  return m_impl->surfaceCreateProc();
}

ContextCreateProc AndroidGraphicsContext::contextCreateProc()
{
  return m_impl->contextCreateProc();
}

void AndroidGraphicsContext::onInitProperties(VGG::layer::ContextProperty& property)
{
  m_impl->onInitProperties(property);
}

} // namespace VGG
