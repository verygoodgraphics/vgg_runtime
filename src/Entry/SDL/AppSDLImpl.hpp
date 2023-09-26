#pragma once

#include "EventAPISDLImpl.hpp"

#include "Application/AppBase.hpp"
#include "Application/Event/EventAPI.hpp"
#include "EventConvert.hpp"
#include "Utility/Log.hpp"

#include "Layer/Graphics/GraphicsContext.hpp"
#include "Layer/Graphics/ContextInfoGL.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>

#include <any>
#include <optional>

namespace VGG::entry
{

using namespace VGG::app;

class AppSDLImpl : public AppBase<AppSDLImpl>
{
  struct SDLState
  {
    SDL_Window* window{ nullptr };
    SDL_GLContext glContext{ nullptr };
  };
  SDLState m_sdlState;
  ContextInfoGL m_glContext;
  using Getter = std::function<std::any(void)>;
  using Setter = std::function<void(std::any)>;
  std::unordered_map<std::string, std::pair<Getter, Setter>> m_properties;

public:
  static inline void handleGLError()
  {
    const char* err = SDL_GetError();
    FAIL("SDL Error: %s", err);
    SDL_ClearError();
  }

#ifdef __linux__
  static double getScaleFactor()
  {
    static constexpr int NVARS = 4;
    static const char* s_vars[NVARS] = {
      "FORCE_SCALE",
      "QT_SCALE_FACTOR",
      "QT_SCREEN_SCALE_FACTOR",
      "GDK_SCALE",
    };
    for (int i = 0; i < NVARS; i++)
    {
      const char* strVal = getenv(s_vars[i]);
      if (strVal)
      {
        double val = atof(strVal);
        if (val >= 1.0)
        {
          return val;
        }
      }
    }
    return 1.0;
  }
#else
  static inline double getScaleFactor()
  {
    return 1.0;
  }
#endif

  void onInitProperties(layer::ContextProperty& property) override
  {
    property.dpiScaling = resolutionScale();
    property.api = layer::EGraphicsAPIBackend::API_OPENGL;
  }

  bool onInit() override
  {
    const auto& cfg = appConfig();
    if (initContext(cfg.windowSize[0],
                    cfg.windowSize[1])) // TODO:: remove hard-coded initial window size
      return false;

    SDL_GL_SetSwapInterval(0);

    // Reigster SDL event impl
    auto eventAPIImpl = std::make_unique<EventAPISDLImpl>();
    EventManager::registerEventAPI(std::move(eventAPIImpl));
    return true;
  }

  void shutdown() override
  {
    if (m_sdlState.glContext)
    {
      // NOTE The failure to delete GL context may be related to multi-thread and is hard
      // to make it right. For simplicity, we can just safely ignore the deletion.
      //
      // SDL_GL_DeleteContext(m_sdlState.glContext);
    }
    if (m_sdlState.window)
    {
      SDL_DestroyWindow(m_sdlState.window);
    }
    SDL_Quit();
  }

  std::optional<AppError> initContext(int w, int h)
  {
    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      handleGLError();
      return AppError(AppError::EKind::RENDER_ENGINE_ERROR, "sdl init failed");
    }
    SDL_version compileVersion, linkVersion;
    SDL_VERSION(&compileVersion);
    SDL_GetVersion(&linkVersion);
    INFO("SDL version %d.%d.%d (linked %d.%d.%d)",
         compileVersion.major,
         compileVersion.minor,
         compileVersion.patch,
         linkVersion.major,
         linkVersion.minor,
         linkVersion.patch);

    // setup opengl properties
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, appConfig().graphicsContextConfig.stencilBit);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, appConfig().graphicsContextConfig.multiSample);
    // SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
#ifndef EMSCRIPTEN
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
#endif

    // create window
    int winWidth = w;
    int winHeight = h;
    SDL_Window* window =
#ifndef EMSCRIPTEN
      SDL_CreateWindow(appName().c_str(),
#else
      SDL_CreateWindow(nullptr,
#endif
                       SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED,
                       winWidth,
                       winHeight,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
      handleGLError();
      return AppError(AppError::EKind::RENDER_ENGINE_ERROR, "Create Window Failed\n");
    }
    m_sdlState.window = window;
    // create context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    m_glContext.context = glContext;
    if (!glContext)
    {
      handleGLError();
      return AppError(AppError::EKind::RENDER_ENGINE_ERROR, "Create Context Failed");
    }

    m_sdlState.glContext = glContext;

    // switch to this context
    auto appResult = makeContextCurrent();
    if (appResult.has_value())
    {
      return appResult;
    }
    INFO("GL_VENDOR: %s", glGetString(GL_VENDOR));
    INFO("GL_RENDERER: %s", glGetString(GL_RENDERER));
    INFO("GL_VERSION: %s", glGetString(GL_VERSION));
    INFO("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return std::nullopt;
  }

public:
  bool makeCurrent() override
  {
    if (makeContextCurrent())
    {
      return false;
    }
    return true;
  }

  bool swap() override
  {
    swapBuffer();
    return true;
  }

  std::optional<AppError> makeContextCurrent()
  {
    if (SDL_GL_MakeCurrent(m_sdlState.window, m_sdlState.glContext) != 0)
    {
      handleGLError();
      return AppError(AppError::EKind::MAKE_CURRENT_CONTEXT_ERROR, "Make Current Context Error");
    }
    return std::nullopt;
  }

  float resolutionScale()
  {
#if defined(VGG_HOST_macOS) || defined(EMSCRIPTEN)
    int dw, dh;
    int ww, wh;
    SDL_GL_GetDrawableSize(this->m_sdlState.window, &dw, &dh);
    SDL_GetWindowSize(this->m_sdlState.window, &ww, &wh);
    const float s = float(dw) / (float)ww;
    return s;
#else
    return getScaleFactor();
#endif
  }

  void pollEvent()
  {
    SDL_Event evt;
    while (SDL_PollEvent(&evt))
    {
      auto event = toUEvent(evt, resolutionScale());
      sendEvent(event);
    }
  }

  void swapBuffer()
  {
    // auto profiler = CappingProfiler::getInstance();

    // // display fps
    // if (profiler)
    // {
    //   SDL_SetWindowTitle(m_sdlState.window, profiler->fpsStr());
    // }

    // swap buffer at last
    SDL_GL_SwapWindow(m_sdlState.window);
  }

  void* contextInfo() override
  {
    return &m_glContext;
  }

  ~AppSDLImpl()
  {
  }
};
} // namespace VGG::entry
