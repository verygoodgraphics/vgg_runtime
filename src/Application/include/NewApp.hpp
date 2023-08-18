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

#include "Event/Event.h"
#include <core/SkColor.h>
#include <core/SkFont.h>
#include <core/SkFontTypes.h>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <queue>
#include <fstream>
#include <memory>
#include <optional>
#include "Scene/VGGLayer.h"
#ifdef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#include <emscripten/emscripten.h>
#else
#endif

#include "Application/include/Event/EventListener.h"
#include "Scene/Scene.h"
#include "Scene/Zoomer.h"
#include "Application/UIView.hpp"
#include "Common/Math.hpp"
#include "CappingProfiler.hpp"
#include "Log.h"

using namespace VGG;
namespace VGGNew
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
 * SDL event struct is adopted as our event handler framework, sdl runtime is not necessary for
 * App NOTE: It must be derived to use this class, and the singleton app instance can be obtained
 * by App::getIntance<DerivedApp>().
 */

/*
 * bool initContext(int, int, const std::string&)
 *	It should guarantee the GL context properly created.
 *	*/
HAS_MEMBER_FUNCTION_DEF(initContext)
/*
 * float getDPIScale()
 *	return current dpi
 *	*/
HAS_MEMBER_FUNCTION_DEF(getDPIScale)
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

struct DPI
{
  inline static double ScaleFactor = 1.0;
};
template<typename T>
class App
{
  // NOLINTBEGIN
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
  // NOLINTEND

protected: // protected members and static members
  static constexpr int N_MULTISAMPLE = 0;
  static constexpr int N_STENCILBITS = 8;
  std::shared_ptr<VLayer> m_layer;
  std::unique_ptr<EventListener> m_eventListener;

  bool m_inited{ false };
  bool m_shouldExit;
  int m_width;
  int m_height;
  double m_pixelRatio{ 1.0 };
  double m_dpiRatio{ 1.0 };
  float m_curMouseX{ 0.f }, m_curMouseY{ 0.f };
  bool m_drawInfo{ false };
  bool m_capture = false;

  static std::optional<AppError> init(App* app, int w, int h, const std::string& title)
  {
    ASSERT(app);
    if (app->m_inited)
    {
      return AppError(AppError::Kind::HasInitError, "app has init");
    }

    app->m_width = w;
    app->m_height = h;

    auto appResult = app->Self()->initContext(w * app->m_pixelRatio, h * app->m_pixelRatio, title);
    if (appResult.has_value())
      return appResult;

    appResult = app->Self()->makeContextCurrent();
    if (appResult.has_value())
      return appResult;

    // get necessary property about window and DPI
    auto drawSize = std::any_cast<std::pair<int, int>>(app->Self()->getProperty("viewport_size"));
    auto winSize = std::any_cast<std::pair<int, int>>(app->Self()->getProperty("window_size"));

#ifdef EMSCRIPTEN
    app->m_pixelRatio = (double)drawSize.first / winSize.first;
#endif
    app->m_dpiRatio = app->Self()->getDPIScale();
    // app->m_zoomer.dpiRatio = DPI::ScaleFactor;

    DEBUG("Drawable size: %d %d", drawSize.first, drawSize.second);
    DEBUG("Window size: %d %d", winSize.first, winSize.second);
    DEBUG("Pixel ratio: %.2lf", app->m_pixelRatio);
    DEBUG("DPI ratio: %.2lf", app->m_dpiRatio);

    // get skia surface and canvas
    // sk_sp<SkSurface> surface = app->setup_skia_surface(w * app->m_pixelRatio * app->m_dpiRatio,
    //                                                    h * app->m_pixelRatio *
    //                                                    app->m_dpiRatio);

    // init capture

    // init m_layer
    app->m_layer = VLayer::makeVLayer(2.0);
    // extra initialization
    app->Self()->onInit();
    app->m_inited = true;
    return std::nullopt;
  }

  App()
    : m_inited(false)
    , m_shouldExit(false)
    , m_width(0)
    , m_height(0)
    , m_pixelRatio(1.0)
  {
  }

protected:
  bool sendEvent(const UEvent& e)
  {
    bool handled = false;
    if (m_eventListener)
    {
      handled = m_eventListener->dispatchEvent(e, this);
    }
    // auto type = evt.type;
    // auto key = evt.key.keysym.sym;
    // auto mod = evt.key.keysym.mod;
    //
    // if (type == SDL_QUIT)
    // {
    //   m_shouldExit = true;
    //   return true;
    // }
    //
    // if (auto& window = evt.window;
    //     type == SDL_WINDOWEVENT && window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    // {
    //   int w = window.data1 / DPI::ScaleFactor;
    //   int h = window.data2 / DPI::ScaleFactor;
    //
    //   DEBUG("Window resizing: (%d %d)", w, h);
    //   // resizeSkiaSurface(w * m_pixelRatio * m_dpiRatio, h * m_pixelRatio * m_dpiRatio);
    //
    //   m_width = w;
    //   m_height = h;
    //
    //   return true;
    // }

    return !handled ? onGlobalEvent(e) : true;
  }

  bool onGlobalEvent(const UEvent& evt)
  {
    // handles App-wide events
    return true;
  }

public: // public methods
  inline bool shouldExit()
  {
    return m_shouldExit;
  }

  inline VLayer* layer()
  {
    return m_layer.get();
  }

  void setEventListener(std::unique_ptr<EventListener> listener)
  {
    m_eventListener = std::move(listener);
  }

  // fps <= 0 indicates rendering as fast as possible
  void exec()
  {
    if (!m_inited)
    {
      return;
    }
    Self()->pollEvent();
    m_layer->beginFrame();
    m_layer->render();
    m_layer->endFrame();
    Self()->swapBuffer();
  }

public: // public static methods
  static T* getInstance(int w = 800,
                        int h = 600,
                        const std::string& title = "App",
                        char** argv = 0,
                        int argc = 0)
  {
    static_assert(std::is_base_of<App<T>, T>());
    static T s_app;
    // if already initialized, these init params are ignored
    if (!s_app.m_inited)
    {
      auto appResult = init(&s_app, w, h, title);
      if (appResult.has_value())
      {
        INFO("%s", appResult.value().text.c_str());
        return nullptr;
      }
    }
    VAppInitEvent e;
    e.type = VGG_APP_INIT;
    e.argv = argv;
    e.argc = argc;
    UEvent evt;
    evt.init = e;
    s_app.sendEvent(evt);
    return &s_app;
  }
}; // class App

}; // namespace VGGNew

#endif // __APP_HPP__
