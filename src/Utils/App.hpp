/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __APP_HPP__
#define __APP_HPP__

#include <SDL2/SDL.h>
#ifdef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#include <emscripten/emscripten.h>
#else
#include <SDL2/SDL_opengl.h>
#endif
#include <skia/include/gpu/gl/GrGLInterface.h>
#include <skia/include/gpu/GrDirectContext.h>
#include <skia/include/core/SkSurface.h>
#include <skia/include/core/SkCanvas.h>
#include <skia/include/core/SkData.h>
#include <skia/include/core/SkImage.h>
#include <skia/include/core/SkSwizzle.h>
#include <skia/include/core/SkTextBlob.h>
#include <skia/include/core/SkTime.h>
#include <skia/include/effects/SkDashPathEffect.h>
#include <skia/src/gpu/gl/GrGLUtil.h>

#include "Utils/Profiler.hpp"
#include "Utils/Types.hpp"
#include "Utils/DPI.hpp"

namespace VGG
{
/** App
 *
 * A singleton app class which provides:
 * 1. A single SDL window with OpenGL/ES context for skia.
 * 2. Auto pixel ratio support.
 * 3. Basic events support.
 *
 * NOTE:
 * It must be derived to use this class, and the singleton app
 * instance can be obtained by App::getIntance<DerivedApp>().
 */
class App : public Uncopyable
{
public: // public data and types
  struct SDLState
  {
    SDL_Window* window{ nullptr };
    SDL_GLContext glContext{ nullptr };
  };
  struct SkiaState
  {
    sk_sp<const GrGLInterface> interface{ nullptr };
    sk_sp<GrDirectContext> grContext{ nullptr };
    sk_sp<SkSurface> surface{ nullptr };

    inline SkCanvas* getCanvas()
    {
      if (surface)
      {
        return surface->getCanvas();
      }
      return nullptr;
    }
  };

protected: // protected members and static members
  static constexpr int N_MULTISAMPLE = 0;
  static constexpr int N_STENCILBITS = 8;

  bool m_inited;
  bool m_shouldExit;
  int m_width;
  int m_height;
  double m_pixelRatio;
  int m_nFrame;
  bool m_enableProfiler;
  double m_timestamp;
  SDLState m_sdlState;
  SkiaState m_skiaState;

private: // private static methods
  static inline void handle_sdl_error()
  {
    const char* err = SDL_GetError();
    FAIL("SDL Error: %s", err);
    SDL_ClearError();
  }

#ifdef __linux__
  static double get_scale_factor()
  {
    static constexpr int NVARS = 4;
    static const char* vars[NVARS] = {
      "FORCE_SCALE",
      "QT_SCALE_FACTOR",
      "QT_SCREEN_SCALE_FACTOR",
      "GDK_SCALE",
    };
    for (int i = 0; i < NVARS; i++)
    {
      const char* strVal = getenv(vars[i]);
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
  static inline double get_scale_factor()
  {
    return 1.0;
  }
#endif

  static bool init(App* app, int w, int h, const std::string& title)
  {
    ASSERT(app);
    if (app->m_inited)
    {
      return true;
    }

    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      handle_sdl_error();
      return false;
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
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, N_STENCILBITS);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, N_MULTISAMPLE);
#ifndef EMSCRIPTEN
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

    // create window
    DPI::ScaleFactor = get_scale_factor();
    int winWidth = w * DPI::ScaleFactor;
    int winHeight = h * DPI::ScaleFactor;
    SDL_Window* window =
#ifndef EMSCRIPTEN
      SDL_CreateWindow(title.c_str(),
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
      handle_sdl_error();
      return false;
    }
    app->m_width = w;
    app->m_height = h;
    app->m_sdlState.window = window;

    // create context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
      handle_sdl_error();
      return false;
    }
    app->m_sdlState.glContext = glContext;

    // switch to this context
    if (SDL_GL_MakeCurrent(window, glContext) != 0)
    {
      handle_sdl_error();
      return false;
    }
    INFO("GL_VENDOR: %s", glGetString(GL_VENDOR));
    INFO("GL_RENDERER: %s", glGetString(GL_RENDERER));
    INFO("GL_VERSION: %s", glGetString(GL_VERSION));
    INFO("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // get skia interface and make opengl context
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    if (!interface)
    {
      FAIL("Failed to make skia opengl interface");
      return false;
    }
    sk_sp<GrDirectContext> grContext = GrDirectContext::MakeGL(interface);
    if (!grContext)
    {
      FAIL("Failed to make skia opengl context.");
      return false;
    }
    app->m_skiaState.interface = interface;
    app->m_skiaState.grContext = grContext;

    // get device pixel ratio
    int dw = 0, dh = 0;
    int ww = 0, wh = 0;
    SDL_GL_GetDrawableSize(window, &dw, &dh);
    SDL_GetWindowSize(window, &ww, &wh);
    DEBUG("Drawable size: %d %d", dw, dh);
    DEBUG("Window size: %d %d", ww, wh);
#ifdef __linux__
    app->m_pixelRatio = DPI::ScaleFactor;
#else
    app->m_pixelRatio = (double)dw / ww;
#endif
    DEBUG("Scale factor: %.2lf", DPI::ScaleFactor);
    DEBUG("Pixel ratio: %.2lf", app->m_pixelRatio);

    // get skia surface and canvas
    sk_sp<SkSurface> surface = app->setup_skia_surface(w, h);
    if (!surface)
    {
      FAIL("Failed to make skia surface.");
      return false;
    }
    app->m_skiaState.surface = surface;
    SkCanvas* canvas = surface->getCanvas();
    if (!canvas)
    {
      FAIL("Failed to get skia canvas.");
      return false;
    }

    // setup sdl configs
    SDL_GL_SetSwapInterval(0);

    // extra initialization
    app->onInit();

    app->m_timestamp = SkTime::GetMSecs();
    app->m_inited = true;
    return true;
  }

private: // private methods
  sk_sp<SkSurface> setup_skia_surface(int w, int h)
  {
    ASSERT(m_skiaState.interface);
    ASSERT(m_skiaState.grContext);
    GrGLFramebufferInfo info;
    GR_GL_GetIntegerv(m_skiaState.interface.get(),
                      GR_GL_FRAMEBUFFER_BINDING,
                      (GrGLint*)&info.fFBOID);

    // color type and info format must be the followings for
    // both OpenGL and OpenGL ES, otherwise it will fail
    SkColorType colorType;
    colorType = kRGBA_8888_SkColorType;
    info.fFormat = GR_GL_RGBA8;

    GrBackendRenderTarget target(m_pixelRatio * w,
                                 m_pixelRatio * h,
                                 N_MULTISAMPLE,
                                 N_STENCILBITS,
                                 info);
    SkSurfaceProps props;
    return SkSurface::MakeFromBackendRenderTarget(m_skiaState.grContext.get(),
                                                  target,
                                                  kBottomLeft_GrSurfaceOrigin,
                                                  colorType,
                                                  nullptr,
                                                  &props);
  }

  void cap_framerate(double fps = 60.)
  {
    double lasting = SkTime::GetMSecs() - m_timestamp;
    double total = 1000. / fps;
    if (lasting < total)
    {
      SDL_Delay(total - lasting);
    }
    m_timestamp = SkTime::GetMSecs();
  }

  bool on_global_event(const SDL_Event& evt)
  {
    auto type = evt.type;
    auto key = evt.key.keysym.sym;
    auto mod = evt.key.keysym.mod;

    if (type == SDL_QUIT)
    {
      m_shouldExit = true;
      return true;
    }

    if (auto& window = evt.window;
        type == SDL_WINDOWEVENT && window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    {
      int w = window.data1 / DPI::ScaleFactor;
      int h = window.data2 / DPI::ScaleFactor;
      m_skiaState.surface = setup_skia_surface(w, h);
      m_width = w;
      m_height = h;
      return true;
    }

    return onGlobalEvent(evt);
  }

protected: // protected methods
  App()
    : m_inited(false)
    , m_shouldExit(false)
    , m_width(0)
    , m_height(0)
    , m_pixelRatio(1.0)
    , m_nFrame(0)
    , m_enableProfiler(false)
  {
  }

  virtual ~App()
  {
    if (m_inited && m_sdlState.glContext)
    {
      // NOTE The failure to delete GL context may be related to multi-thread and is hard
      // to make it right. For simplicity, we can just safely ignore the deletion.
      //
      // SDL_GL_DeleteContext(m_sdlState.glContext);
    }
    if (m_inited && m_sdlState.window)
    {
      SDL_DestroyWindow(m_sdlState.window);
    }
    SDL_Quit();
  }

  virtual void onInit()
  {
  }

  virtual void onFrame()
  {
  }

  virtual void onEvent(const SDL_Event& evt)
  {
  }

  // TODO support shortcuts of combined keys
  virtual bool onGlobalEvent(const SDL_Event& evt)
  {
    return false;
  }

  inline SkCanvas* getCanvas()
  {
    return m_skiaState.getCanvas();
  }

public: // public methods
  inline bool shouldExit()
  {
    return m_shouldExit;
  }

  void frame()
  {
    if (!m_inited)
    {
      return;
    }
#if 0
    INFO("frame %d", m_nFrame++);
#endif

    // deal with events
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) // TODO change to WaitEvent to reduce CPU usage
    {
      // process global events like shortcuts first
      if (on_global_event(evt))
      {
        continue;
      }

      // process app events at last
      onEvent(evt);
    }

    // get and setup canvas
    SkCanvas* canvas = getCanvas();
    ASSERT(canvas);
    canvas->save();
    canvas->clear(SK_ColorWHITE);
    canvas->scale(m_pixelRatio, m_pixelRatio);

    // setup profiler
    Profiler* profiler = nullptr;
    if (m_enableProfiler)
    {
      profiler = Profiler::getInstance();
      ASSERT(profiler);
      profiler->startTiming("paint");
    }

    // update frame
    onFrame();

    // and cap frame rate if necessary
    cap_framerate();

    // finish this frame
    if (profiler && m_enableProfiler)
    {
      profiler->stopTiming("paint");
      profiler->startTiming("flush");
    }
    canvas->flush();
    if (profiler && m_enableProfiler)
    {
      profiler->stopTiming("flush");
      profiler->nextMeasure();
    }
    canvas->restore();

    // swap buffer at last
    SDL_GL_SwapWindow(m_sdlState.window);
  }

public: // public static methods
  template<typename DerivedApp>
  static DerivedApp* getInstance(int w = 800, int h = 600, const std::string& title = "App")
  {
    static_assert(std::is_base_of<App, DerivedApp>());

    static DerivedApp app;

    // if already initialized, these init params are ignored
    if (!init(&app, w, h, title))
    {
      return nullptr;
    }
    return &app;
  }

}; // class App

}; // namespace VGG

#endif // __APP_HPP__
