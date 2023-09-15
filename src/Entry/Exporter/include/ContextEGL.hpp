#pragma once

#include <Application/interface/AppBase.hpp>
#include <EGL/egl.h>
#include <Core/FontManager.h>
#include <Core/PaintNode.h>
#include <fstream>
#include <Application/interface/Event/EventListener.h>
#include <Scene/GraphicsContext.h>
#include <Scene/GraphicsLayer.h>
#include <Scene/VGGLayer.h>
#include <algorithm>
#include <exception>
#include <memory>
#include <optional>
#include <string>
namespace VGG::exporter
{

using namespace VGG::app;

constexpr int OPENGL_VERSION[] = { 4, 5 };
constexpr int PIXEL_BUFFER_WIDTH = 800;
constexpr int PIXEL_BUFFER_HEIGHT = 600;
constexpr EGLint CONFIG_ATTRIBS[] = { EGL_SURFACE_TYPE,
                                      EGL_PBUFFER_BIT,
                                      EGL_BLUE_SIZE,
                                      8,
                                      EGL_GREEN_SIZE,
                                      8,
                                      EGL_RED_SIZE,
                                      8,
                                      EGL_DEPTH_SIZE,
                                      8,
                                      EGL_RENDERABLE_TYPE,
                                      EGL_OPENGL_BIT,
                                      EGL_NONE };

constexpr EGLint CONTEXT_ATTRIBS[] = { EGL_CONTEXT_MAJOR_VERSION,
                                       OPENGL_VERSION[0],
                                       EGL_CONTEXT_MINOR_VERSION,
                                       OPENGL_VERSION[1],
                                       EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                       EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                       EGL_NONE };

const char* getEglErrorInfo(EGLint error)
{
  switch (error)
  {
    case EGL_NOT_INITIALIZED:
      return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
      return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
      return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
      return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONTEXT:
      return "EGL_BAD_CONTEXT";
    case EGL_BAD_CONFIG:
      return "EGL_BAD_CONFIG";
    case EGL_BAD_CURRENT_SURFACE:
      return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
      return "EGL_BAD_DISPLAY";
    case EGL_BAD_SURFACE:
      return "EGL_BAD_SURFACE";
    case EGL_BAD_MATCH:
      return "EGL_BAD_MATCH";
    case EGL_BAD_PARAMETER:
      return "EGL_BAD_PARAMETER";
    case EGL_BAD_NATIVE_PIXMAP:
      return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
      return "EGL_BAD_NATIVE_WINDOW";
    case EGL_CONTEXT_LOST:
      return "EGL_CONTEXT_LOST";
    case EGL_SUCCESS:
      return "NO ERROR";
    default:
      return "UNKNOWN ERROR";
  }
}

#define EGL_CHECK()                                                                                \
  do                                                                                               \
  {                                                                                                \
    auto error = eglGetError();                                                                    \
    if (error != EGL_SUCCESS)                                                                      \
    {                                                                                              \
      WARN("EGL Error: %s", getEglErrorInfo(error));                                               \
    }                                                                                              \
  } while (0)

class EGLGraphicsContext : public layer::GraphicsContext
{
  EGLContext m_eglCtx;
  EGLSurface m_eglSurface;
  EGLDisplay m_eglDisplay;
  EGLConfig m_eglConfig = nullptr;
  std::unordered_map<std::string, std::any> m_properties;

private:
  void resizeEGLSurface(int w, int h)
  {

    EGLint width = w;
    EGLint height = h;

    eglDestroySurface(m_eglDisplay, m_eglSurface);
    EGL_CHECK();
    EGLint pixelBufferAttribs[] = {
      EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE,
    };
    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConfig, pixelBufferAttribs);
    EGL_CHECK();

    eglBindAPI(EGL_OPENGL_API);
    // Make the context current with the new surface
    makeContextCurrent();
    EGL_CHECK();
  }

public:
  float getDPIScale()
  {
    return 1.0;
  }
  std::optional<AppError> initContext(int w, int h)
  {
    int winWidth = w;
    int winHeight = h;

    m_properties["window_size"] = std::pair{ winWidth, winHeight };
    m_properties["viewport_size"] = std::pair{ winWidth, winHeight };
    m_properties["app_size"] = std::pair{ winWidth, winHeight };

    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_eglDisplay == EGL_NO_DISPLAY)
    {
      WARN("No default display");
      // try EXT_platform_device, see
      // https://www.khronos.org/registry/EGL/extensions/EXT/EGL_EXT_platform_device.txt
      // egl_display = create_display_from_device();
      return AppError(AppError::EKind::EGL_NO_DISPLAY_ERROR, "No default display");
    }

    EGLint major = 0, minor = 0;
    eglInitialize(m_eglDisplay, &major, &minor);
    INFO("EGL version: %d, %d", major, minor);
    INFO("EGL vendor string: %s", eglQueryString(m_eglDisplay, EGL_VENDOR));
    EGL_CHECK();

    // 2. Select an appropriate configuration
    EGLint numConfigs = 0;
    eglChooseConfig(m_eglDisplay, CONFIG_ATTRIBS, &m_eglConfig, 1, &numConfigs);
    EGL_CHECK();

    int maxSurfaceSize[2];
    if (eglGetConfigAttrib(m_eglDisplay, m_eglConfig, EGL_MAX_PBUFFER_WIDTH, maxSurfaceSize) !=
        EGL_TRUE)
    {
      EGL_CHECK();
      return AppError(AppError::EKind::EGL_GET_ATTRIB_ERROR, "get attribute error");
    }
    if (eglGetConfigAttrib(m_eglDisplay, m_eglConfig, EGL_MAX_PBUFFER_HEIGHT, maxSurfaceSize + 1) !=
        EGL_TRUE)
    {
      EGL_CHECK();
      return AppError(AppError::EKind::EGL_GET_ATTRIB_ERROR, "get attribute error");
    }

    if (winWidth + 100 > maxSurfaceSize[0] || winHeight + 100 > maxSurfaceSize[1])
    {
      return AppError(AppError::EKind::TEXTURE_SIZE_OUT_OF_RANGE_ERROR, "Texture size out of range");
    }

    // 3. Create a surface

    EGLint pixelBufferAttribs[] = {
      EGL_WIDTH, w, EGL_HEIGHT, h, EGL_NONE,
    };
    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConfig, pixelBufferAttribs);
    EGL_CHECK();

    // 4. Bind the API
    eglBindAPI(EGL_OPENGL_API);
    EGL_CHECK();

    // 5. Create a context and make it current
    m_eglCtx = eglCreateContext(m_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, CONTEXT_ATTRIBS);
    EGL_CHECK();

    return makeContextCurrent();
  }
  std::optional<AppError> makeContextCurrent()
  {
    if (eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglCtx) == EGL_TRUE)
    {
      return std::nullopt;
    }
    EGL_CHECK();
    return AppError(AppError::EKind::MAKE_CURRENT_CONTEXT_ERROR, "make current failed");
  }

  bool swap() override
  {
    swapBuffer();
    return true;
  }

  bool makeCurrent() override
  {
    return true;
  }

  void* contextInfo() override
  {
    return m_eglCtx;
  }

  bool onResize(int w, int h) override
  {
    return true;
  }

  virtual void shutdown() override
  {
    destoryEGLContext();
  }

  void onInitProperties(layer::ContextProperty& property) override
  {
    property.api = layer::EGraphicsAPIBackend::API_OPENGL;
    // No property need to be init in EGL
  }

  bool onInit() override
  {
    const auto& cfg = config();
    if (initContext(cfg.windowSize[0], cfg.windowSize[1]))
      return false;
    return true;
  }

  void pollEvent()
  {
    // no event need to deal with
  }

  void swapBuffer()
  {
    if (eglSwapBuffers(m_eglDisplay, m_eglSurface) == EGL_FALSE)
    {
      WARN("egl swap buffer failed\n");
    }
  }

  void destoryEGLContext()
  {
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_eglDisplay, m_eglCtx);
    eglDestroySurface(m_eglDisplay, m_eglSurface);
    eglTerminate(m_eglDisplay);
    // eglReleaseThread();
  }

  ~EGLGraphicsContext()
  {
    destoryEGLContext();
  }
};

} // namespace VGG::exporter
