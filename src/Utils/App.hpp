/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
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

#include "gpu/GrTypes.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#ifdef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#include <emscripten/emscripten.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#define GR_GL_LOG_CALLS 0
#define GR_GL_CHECK_ERROR 0
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkPicture.h>
#include <include/core/SkPictureRecorder.h>
#include <include/core/SkImage.h>
#include <include/core/SkSwizzle.h>
#include <include/core/SkTextBlob.h>
#include <include/core/SkTime.h>
#include <include/effects/SkDashPathEffect.h>
#include <src/gpu/ganesh/gl/GrGLDefines.h>
#include <src/gpu/ganesh/gl/GrGLUtil.h>
#include "include/gpu/gl/GrGLFunctions.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "encode/SkPngEncoder.h"

#include "Utils/CappingProfiler.hpp"
#include "Utils/Types.hpp"
#include "Utils/DPI.hpp"
#include "Utils/FileManager.hpp"
#include "Utils/Scheduler.hpp"
#include "Scene/Scene.h"
#include "View/UIView.hpp"

namespace VGG
{

#define HAS_MEMBER_FUNCTION_DEF(FUNC)                                                              \
  template<typename T>                                                                             \
  class has_member_##FUNC                                                                          \
  {                                                                                                \
    typedef char one;                                                                              \
    struct two                                                                                     \
    {                                                                                              \
      char x[2];                                                                                   \
    };                                                                                             \
    template<typename C>                                                                           \
    static one test(decltype(&C::FUNC));                                                           \
    template<typename C>                                                                           \
    static two test(...);                                                                          \
                                                                                                   \
  public:                                                                                          \
    enum                                                                                           \
    {                                                                                              \
      value = sizeof(test<T>(0)) == sizeof(char)                                                   \
    };                                                                                             \
  };

/** App
 *
 * A singleton app class which provides:
 * 1. A single OpenGL/ES context for skia.
 * 2. Auto pixel ratio support.
 * 3. Basic events support.
 *
 * CRTP: the following methods must be implemented in T to integrate custome window manager
 * SDL event struct is adopted as our event handler framework, sdl runtime is not necessary for App
 * NOTE:
 * It must be derived to use this class, and the singleton app
 * instance can be obtained by App::getIntance<DerivedApp>().
 */

/*
 * bool initContext(int, int, const std::string&)
 *	It should guarantee the GL context properly created.
 *	*/
HAS_MEMBER_FUNCTION_DEF(initContext)
/*
 * bool makeContextCurrent()
 *	It should switch the GL context in current thread after invoke.
 */
HAS_MEMBER_FUNCTION_DEF(makeContextCurrent)
/*
 * std::any setProperty(const std::string &, std::any value):
 *	It should return the properties about the T
 *	"window_size", "app_size", "viewport_size"
 *
 * */
HAS_MEMBER_FUNCTION_DEF(getProperty)
/*
 *	"window_size": window size
 *	"viewport_size": viewport size
 *	"app_size": just caches the command line input (consistent with window_size in most cases,
 *	depend on backends)
 * */
HAS_MEMBER_FUNCTION_DEF(setProperty)

/*
 * void swapBuffer():
 *	swaps the buffer
 * */
HAS_MEMBER_FUNCTION_DEF(swapBuffer)

/*
 * void onInit():
 *	Invoked after all init processes complete.
 * */
HAS_MEMBER_FUNCTION_DEF(onInit)

/*
 * void pollEvent()
 * Polls the backends events
 * */
HAS_MEMBER_FUNCTION_DEF(pollEvent)

#undef HAS_MEMBER_FUNCTION_DEF

struct AppError
{
  enum class Kind
  {
    TextureSizeOutOfRangeError,
    EGLNoDisplayError,
    EGLGetAttribError,
    MakeCurrentContextError,
    HasInitError,
    UnknownError,
    RenderEngineError,
  };
  Kind kind;
  std::string text;
  AppError(Kind k, const std::string& error)
    : kind(k)
    , text(error)
  {
  }
};

template<typename T>
class App : public Uncopyable
{
private:
  inline T* Self()
  {
    static_assert(std::is_base_of<App, T>::value);
    static_assert(has_member_initContext<T>::value);
    static_assert(has_member_makeContextCurrent<T>::value);
    static_assert(has_member_getProperty<T>::value);
    static_assert(has_member_setProperty<T>::value);
    static_assert(has_member_swapBuffer<T>::value);
    static_assert(has_member_onInit<T>::value);
    static_assert(has_member_pollEvent<T>::value);
    return static_cast<T*>(this);
  }

public: // public data and types
  struct SkiaState
  {
    sk_sp<const GrGLInterface> interface {
      nullptr
    };
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

  struct Zoomer
  {
    static constexpr double minZoom = 0.01;
    static constexpr double maxZoom = 10.0;
    Vec2 offset{ 0.0, 0.0 };
    double zoom{ 1.0 };
    bool panning{ false };

    void apply(SkCanvas* canvas)
    {
      ASSERT(canvas);
      canvas->translate(offset.x, offset.y);
      canvas->scale(zoom, zoom);
    }

    void restore(SkCanvas* canvas)
    {
      ASSERT(canvas);
      canvas->scale(1. / zoom, 1. / zoom);
      canvas->translate(-offset.x, -offset.y);
    }

    SDL_Event mapEvent(SDL_Event evt, double scaleFactor)
    {
      if (evt.type == SDL_MOUSEMOTION)
      {
        double x1 = evt.motion.x;
        double y1 = evt.motion.y;
        double x0 = x1 - evt.motion.xrel;
        double y0 = y1 - evt.motion.yrel;
        x1 = (x1 / scaleFactor - offset.x) / zoom;
        y1 = (y1 / scaleFactor - offset.y) / zoom;
        x0 = (x0 / scaleFactor - offset.x) / zoom;
        y0 = (y0 / scaleFactor - offset.y) / zoom;
        evt.motion.x = FLOAT2SINT(x1);
        evt.motion.y = FLOAT2SINT(y1);
        evt.motion.xrel = FLOAT2SINT(x1 - x0);
        evt.motion.yrel = FLOAT2SINT(y1 - y0);
      }
      return evt;
    }
  };

public:
  inline void setOnFrameOnce(std::function<void()>&& cb)
  {
    Scheduler::setOnFrameOnce(std::move(cb));
  }

  inline void startRunMode()
  {
    EntityManager::setInteractionMode(EntityManager::InteractionMode::RUN);
    InputManager::setMouseCursor(MouseEntity::CursorType::NORMAL);
    if (auto container = EntityManager::getEntities())
    {
      container->setRunModeInteractions();
    }
  }

protected: // protected members and static members
  static constexpr int N_MULTISAMPLE = 0;
  static constexpr int N_STENCILBITS = 8;

  bool m_inited{ false };
  bool m_shouldExit;
  int m_width;
  int m_height;
  double m_pixelRatio{ 1.0 };
  int m_nFrame;
  double m_timestamp;
  SkiaState m_skiaState;
  Zoomer m_zoomer;
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Scene> m_scene;
  bool m_useOldRenderer = true;
  std::unique_ptr<SkPictureRecorder> m_recorder;
  bool m_capture = false;

  std::function<void(Scene* scene, int type)> m_reloadCallback;

  static std::optional<AppError> init(App* app, int w, int h, const std::string& title)
  {
    ASSERT(app);
    if (app->m_inited)
    {
      return AppError(AppError::Kind::HasInitError, "app has init");
    }

    auto appResult = app->Self()->initContext(w * app->m_pixelRatio, h * app->m_pixelRatio, title);
    if (appResult.has_value())
      return appResult;

    appResult = app->Self()->makeContextCurrent();
    if (appResult.has_value())
      return appResult;

    appResult = app->updateSkiaEngine();
    if (appResult.has_value())
      return appResult;

    // get necessary property about window and DPI
    auto drawSize = std::any_cast<std::pair<int, int>>(app->Self()->getProperty("viewport_size"));
    auto winSize = std::any_cast<std::pair<int, int>>(app->Self()->getProperty("window_size"));

    DEBUG("Drawable size: %d %d", drawSize.first, drawSize.second);
    DEBUG("Window size: %d %d", winSize.first, winSize.second);
    DEBUG("Scale factor: %.2lf", DPI::ScaleFactor);
    DEBUG("Pixel ratio: %.2lf", app->m_pixelRatio);

    // get skia surface and canvas
    sk_sp<SkSurface> surface =
      app->setup_skia_surface(w * app->m_pixelRatio, h * app->m_pixelRatio);
    if (!surface)
    {
      return AppError(AppError::Kind::RenderEngineError, "Failed to make skia surface");
    }
    app->m_skiaState.surface = surface;
    SkCanvas* canvas = surface->getCanvas();
    if (!canvas)
    {
      return AppError(AppError::Kind::RenderEngineError, "Failed to make skia canvas");
    }

    // init capture

    app->m_recorder = std::make_unique<SkPictureRecorder>();
    // extra initialization
    app->Self()->onInit();
    app->m_timestamp = SkTime::GetMSecs();
    app->m_inited = true;
    return std::nullopt;
  }

private: // private methods
  sk_sp<SkSurface> create_skia_surface2(int w, int h)
  {
    ASSERT(m_skiaState.interface);
    ASSERT(m_skiaState.grContext);
    SkImageInfo info = SkImageInfo::Make(w,
                                         h,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    auto gpuSurface =
      SkSurfaces::RenderTarget(m_skiaState.grContext.get(), skgpu::Budgeted::kNo, info);
    if (!gpuSurface)
    {
      return nullptr;
    }
    return gpuSurface;
  }
  sk_sp<SkSurface> setup_skia_surface(int w, int h)
  {

    ASSERT(m_skiaState.interface);
    ASSERT(m_skiaState.grContext);
    GrGLFramebufferInfo info;
    info.fFBOID = 0;
    // GR_GL_GetIntegerv(m_skiaState.interface.get(),
    //                   GR_GL_FRAMEBUFFER_BINDING,
    //                   (GrGLint*)&info.fFBOID);

    // color type and info format must be the followings for
    // both OpenGL and OpenGL ES, otherwise it will fail
    info.fFormat = GR_GL_RGBA8;
    GrBackendRenderTarget target(w, h, N_MULTISAMPLE, N_STENCILBITS, info);

    SkSurfaceProps props;
    return SkSurfaces::WrapBackendRenderTarget(m_skiaState.grContext.get(),
                                               target,
                                               kBottomLeft_GrSurfaceOrigin,
                                               SkColorType::kRGBA_8888_SkColorType,
                                               nullptr,
                                               &props);
  }

protected: // protected methods
  App()
    : m_inited(false)
    , m_shouldExit(false)
    , m_width(0)
    , m_height(0)
    , m_pixelRatio(1.0)
    , m_nFrame(0)
  {
  }

  void resizeSkiaSurface(int w, int h)
  {
    m_skiaState.surface = setup_skia_surface(w, h);
    m_width = w;
    m_height = h;
  }

  std::optional<AppError> updateSkiaEngine()
  {
    // Create Skia
    // get skia interface and make opengl context
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    if (!interface)
    {
      return AppError(AppError::Kind::RenderEngineError, "Failed to make skia opengl interface");
    }
    sk_sp<GrDirectContext> grContext = GrDirectContext::MakeGL(interface);
    if (!grContext)
    {
      return AppError(AppError::Kind::RenderEngineError, "Failed to make skia opengl context.");
    }
    m_skiaState.interface = interface;
    m_skiaState.grContext = grContext;
    return std::nullopt;
  }

  bool dispatchGlobalEvent(const SDL_Event& evt)
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

      DEBUG("Window resizing: (%d %d)", w, h);
      resizeSkiaSurface(w, h);
      return true;
    }

    return onGlobalEvent(evt);
  }

  void onFrame()
  {
    Scheduler::callOnFrameOnce();

    SkCanvas* canvas = nullptr;
    if (m_capture)
    {
      canvas = getCaptureCanvas();
    }
    else
    {
      canvas = getCanvas();
    }
    if (canvas)
    {
      m_zoomer.apply(canvas);
      if (m_useOldRenderer)
      {
        EntityManager::map([&](Entity& entity) { RenderSystem::drawEntity(canvas, entity); });
      }
      if (m_scene)
      {
        m_scene->render(canvas);
      }
      InputManager::draw(canvas);
      m_zoomer.restore(canvas);
    }

    if (m_capture)
    {
      endCapture("picture.skp");
      m_capture = false;
    }

    InputManager::onFrame();
  }

  void dispatchEvent(const SDL_Event& evt)
  {
    auto e = m_zoomer.mapEvent(evt, DPI::ScaleFactor);
    InputManager::onEvent(e);
    m_view->onEvent(e);
  }

  bool onGlobalEvent(const SDL_Event& evt)
  {
    auto type = evt.type;

    if (type == SDL_WINDOWEVENT)
    {
      if (evt.window.event == SDL_WINDOWEVENT_ENTER)
      {
        InputManager::setCursorVisibility(true);
        return true;
      }
      else if (evt.window.event == SDL_WINDOWEVENT_LEAVE)
      {
        InputManager::setCursorVisibility(false);
        return true;
      }
    }

    auto& panning = m_zoomer.panning;
    if (!panning && type == SDL_MOUSEBUTTONDOWN &&
        (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_SPACE]))
    {
      panning = true;
      InputManager::setMouseCursor(MouseEntity::CursorType::MOVE);
      return true;
    }
    else if (panning && type == SDL_MOUSEBUTTONUP)
    {
      panning = false;
      InputManager::setMouseCursor(MouseEntity::CursorType::NORMAL);
      return true;
    }
    else if (panning && type == SDL_MOUSEMOTION)
    {
      m_zoomer.offset.x += evt.motion.xrel / DPI::ScaleFactor;
      m_zoomer.offset.y += evt.motion.yrel / DPI::ScaleFactor;
      return true;
    }
    else if (type == SDL_MOUSEWHEEL && (SDL_GetModState() & KMOD_CTRL))
    {
      int mx, my;
      SDL_GetMouseState(&mx, &my);
      double dz = (evt.wheel.y > 0 ? 1.0 : -1.0) * 0.03;
      double z2 = m_zoomer.zoom * (1 + dz);
      if (z2 > 0.01 && z2 < 100)
      {
        m_zoomer.offset.x -= (mx / DPI::ScaleFactor - m_zoomer.offset.x) * dz;
        m_zoomer.offset.y -= (my / DPI::ScaleFactor - m_zoomer.offset.y) * dz;
        m_zoomer.zoom += m_zoomer.zoom * dz;
        return true;
      }
      return false;
    }
    else if (type == SDL_KEYDOWN)
    {
      auto key = evt.key.keysym.sym;
      auto mod = evt.key.keysym.mod;

      if (key == SDLK_PAGEUP && (SDL_GetModState() & KMOD_CTRL))
      {
        FileManager::prevPage();
        m_scene->preArtboard();
        return true;
      }

      if (key == SDLK_PAGEDOWN && (SDL_GetModState() & KMOD_CTRL))
      {
        FileManager::nextPage();
        m_scene->nextArtboard();
        return true;
      }

      if (key == SDLK_o)
      {
        m_useOldRenderer = !m_useOldRenderer;
        return true;
      }

      if (key == SDLK_c)
      {
        m_capture = true;
      }

      if (key == SDLK_b)
      {
        Scene::enableDrawDebugBound(!Scene::isEnableDrawDebugBound());
        return true;
      }

      if (key == SDLK_DOWN)
      {
        m_reloadCallback(m_scene.get(), 0);
        return true;
      }

      if (key == SDLK_UP)
      {
        m_reloadCallback(m_scene.get(), 1);
        return true;
      }

      if (key == SDLK_s)
      {
        auto image = makeImageSnapshot();

        SkPngEncoder::Options opt;
        opt.fZLibLevel = 0;
        if (auto data = SkPngEncoder::Encode(getDirectContext(), image.get(), opt))
        {
          std::ofstream ofs("snapshot.png", std::ios::binary);
          if (ofs.is_open())
          {
            ofs.write((const char*)data->bytes(), data->size());
          }
        }
        return true;
      }

#ifndef EMSCRIPTEN
      if ((mod & KMOD_CTRL) && key == SDLK_q)
      {
        m_shouldExit = true;
        return true;
      }
#endif
    }

    return false;
  }

public: // public methods
  inline bool shouldExit()
  {
    return m_shouldExit;
  }

  inline SkCanvas* getCanvas()
  {
    return m_skiaState.getCanvas();
  }

  inline SkSurface* getSurface()
  {
    return m_skiaState.surface.get();
  }

  void setView(std::shared_ptr<UIView> view)
  {
    m_view = view;
  }

  inline GrDirectContext* getDirectContext()
  {
    return m_skiaState.grContext.get();
  }

  void setScene(std::shared_ptr<Scene> scene)
  {
    m_scene = scene;
  }

  void setScale(float scale)
  {
    m_pixelRatio = scale;
  }

  void setReloadCallback(std::function<void(Scene*, int)> callback)
  {
    m_reloadCallback = callback;
  }

  sk_sp<SkImage> makeImageSnapshot()
  {
    return m_skiaState.surface->makeImageSnapshot();
  }

  void setUseOldRenderer(bool use)
  {
    m_useOldRenderer = use;
  }

  bool useOldRenderer() const
  {
    return m_useOldRenderer;
  }

  void endCapture(const std::string& fileName)
  {
    auto picture = m_recorder->finishRecordingAsPicture();
    SkFILEWStream stream(fileName.c_str());
    picture->serialize(&stream);
  }

  SkCanvas* getCaptureCanvas()
  {
    m_recorder->beginRecording(m_width, m_height);
    return m_recorder->getRecordingCanvas();
  }

  Scene* getScene()
  {
    return m_scene.get();
  }

  // fps <= 0 indicates rendering as fast as possible
  void frame(int fps)
  {
    if (!m_inited)
    {
      return;
    }
#if 0
	INFO("frame %d", m_nFrame++);
#endif

    Self()->pollEvent();
    // deal with events

    // cap the frame rate
    auto profiler = CappingProfiler::getInstance();
    if (fps > 0)
    {
      if (!(profiler->enoughFrameDuration(fps)))
      {
        return;
      }
    }
    profiler->markFrame();

    // get and setup canvas
    SkCanvas* canvas = getCanvas();
    ASSERT(canvas);
    canvas->save();
    canvas->clear(SK_ColorWHITE);
    canvas->scale(m_pixelRatio, m_pixelRatio);

    // update frame
    onFrame();

    // finish this frame
    canvas->flush();
    canvas->restore();

    Self()->swapBuffer();
  }

public: // public static methods
  static T* getInstance(int w = 800, int h = 600, const std::string& title = "App")
  {
    static_assert(std::is_base_of<App<T>, T>());

    static T app;
    // if already initialized, these init params are ignored
    auto appResult = init(&app, w, h, title);
    if (appResult.has_value())
    {
      INFO("%s", appResult.value().text.c_str());
      return nullptr;
    }
    return &app;
  }

  static std::variant<T*, AppError> createInstance(int w, int h, float scale)
  {
    T* app = new T();
    if (!app)
    {
      return nullptr;
    }
    app->m_pixelRatio = scale;
    auto appResult = init(app, w, h, "instance");
    if (appResult.has_value())
    {
      return appResult.value();
    }
    return app;
  }

  static void destoryInstance(T* app)
  {
    if (app)
      delete app;
  }

}; // class App

}; // namespace VGG

#endif // __APP_HPP__
