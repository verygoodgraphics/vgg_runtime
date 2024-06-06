#pragma once

#include "Entry/SDL/EventAPISDLImpl.hpp"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_opengl.h>
#include <SDL_video.h>

#include <functional>

// A minimal sdl window for test
struct Loop
{
  struct SDLState
  {
    SDL_Window*   window{ nullptr };
    SDL_GLContext glContext{ nullptr };
    SDLState(const char* appName, int w, int h)
    {
      window =
#ifndef EMSCRIPTEN
        SDL_CreateWindow(
          appName,
#else
        SDL_CreateWindow(
          nullptr,
#endif
          SDL_WINDOWPOS_CENTERED,
          SDL_WINDOWPOS_CENTERED,
          w,
          h,
          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
      if (!window)
      {
        handleGLError();
        throw std::runtime_error("SDL_CreateWindow failed");
      }

      // create context
      glContext = SDL_GL_CreateContext(window);
      if (!glContext)
      {
        handleGLError();
        throw std::runtime_error("SDL_CreateWindow failed");
      }
    }

    void swap()
    {
      SDL_GL_SwapWindow(window);
    }

    ~SDLState()
    {
      if (glContext)
      {
        // NOTE The failure to delete GL context may be related to multi-thread and is hard
        // to make it right. For simplicity, we can just safely ignore the deletion.
        //
        // SDL_GL_DeleteContext(m_sdlState.glContext);
      }
      if (window)
      {
        SDL_DestroyWindow(window);
      }
    }
  };
  using EventCallback = std::function<void(const SDL_Event& e, void* userData)>;
  struct Config
  {
    int multiSample = 0;
    int stencilBit = 8;
    int windowSize[2] = { 800, 600 };
  } config;
  std::unique_ptr<SDLState> sdlState;
  const char*               appName = nullptr;
  bool                      exitFlags = false;

  EventCallback eventCallback;

  Loop(const char* appName, int w, int h, const Config& config)
    : appName(appName)
    , config(config)
  {
    init(w, h);
  }

  void setEventCallback(EventCallback callback)
  {
    eventCallback = callback;
  }

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
    static const char*   s_vars[NVARS] = {
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

  bool init(int w, int h)
  {
    initContext(w, h);
    SDL_GL_SetSwapInterval(0);
    // Reigster SDL event impl
    auto eventAPIImpl = std::make_unique<EventAPISDLImpl>();
    EventManager::registerEventAPI(std::move(eventAPIImpl));
    return true;
  }

  ~Loop()
  {
    shutdown();
  }

  void shutdown()
  {
    sdlState.reset();
    SDL_Quit();
  }

  const Config& getConfig() const
  {
    return config;
  }

  const char* initContext(int w, int h)
  {
    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      handleGLError();
      return "sdl init failed";
    }
    SDL_version compileVersion, linkVersion;
    SDL_VERSION(&compileVersion);
    SDL_GetVersion(&linkVersion);
    INFO(
      "SDL version %d.%d.%d (linked %d.%d.%d)",
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
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, config.stencilBit);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, config.multiSample > 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, config.multiSample);
    // SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
#if defined(EMSCRIPTEN) || defined(VGG_TARGET_ARCH_RISCV)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    // create state

    sdlState = std::make_unique<SDLState>(appName, w, h);

    // switch to this context
    auto appResult = makeContextCurrent();
    if (appResult)
    {
      return appResult;
    }

    INFO("GL_VENDOR: %s", glGetString(GL_VENDOR));
    INFO("GL_RENDERER: %s", glGetString(GL_RENDERER));
    INFO("GL_VERSION: %s", glGetString(GL_VERSION));
    INFO("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    return nullptr;
  }

  bool makeCurrent()
  {
    if (makeContextCurrent())
    {
      return false;
    }
    return true;
  }

  void swap()
  {
    sdlState->swap();
  }

  const char* makeContextCurrent()
  {
    if (SDL_GL_MakeCurrent(sdlState->window, sdlState->glContext) != 0)
    {
      handleGLError();
      return "Make Current Context Error\n";
    }
    return nullptr;
  }

  float resolutionScale()
  {
#if defined(VGG_TARGET_PLATFORM_macOS) || defined(EMSCRIPTEN)
    int dw, dh;
    int ww, wh;
    SDL_GL_GetDrawableSize(this->m_sdlState->window, &dw, &dh);
    SDL_GetWindowSize(this->m_sdlState->window, &ww, &wh);
    const float s = float(dw) / (float)ww;
    return s;
#else
    return getScaleFactor();
#endif
  }

  void pollEvent(int timeout)
  {
    SDL_Event evt;
#ifdef EMSCRIPTEN
    while (SDL_PollEvent(&evt))
#else
    while (SDL_WaitEventTimeout(&evt, timeout))
#endif
    {
      if (eventCallback)
      {
        eventCallback(evt, nullptr);
      }
      if (evt.type == SDL_QUIT)
      {
        exitFlags = true;
        break;
      }
    }
  }

  bool exit()
  {
    return exitFlags;
  }
};
