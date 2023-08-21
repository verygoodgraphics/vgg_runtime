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

#include <cstdio>
#include <filesystem>
#include <functional>
#include <queue>
#include <fstream>
#include <memory>
#include <optional>
#include <regex>
#include <any>
#include "Scene/VGGLayer.h"
#include "Application/RenderAdapter.h"
#include "Application/interface/Event/EventListener.h"
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
 * bool initContext(int, int)
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

struct VideoConfig
{
  int stencilBit{ 8 };
  int multiSample{ 0 };
};

struct AppConfig
{
  VideoConfig videoConfig;
  std::string appName;
  int windowSize[2] = { 1920, 1080 };
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
  std::unique_ptr<EventDispatcherLayer> m_layerAdapter;
  std::unique_ptr<EventListener> m_eventListener;
  std::string m_appName;
  AppConfig m_appConfig;
  bool m_inited{ false };
  bool m_shouldExit;
  int m_width;
  int m_height;
  double m_pixelRatio{ 1.0 };

  static std::optional<AppError> init(App* app, const AppConfig& cfg)
  {
    ASSERT(app);
    if (app->m_inited)
    {
      return AppError(AppError::Kind::HasInitError, "app has init");
    }
    app->m_appConfig = cfg;
    int w = app->m_appConfig.windowSize[0];
    int h = app->m_appConfig.windowSize[1];
    auto appResult = app->Self()->initContext(w * app->m_pixelRatio, h * app->m_pixelRatio);
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

    DEBUG("Drawable size: %d %d", drawSize.first, drawSize.second);
    DEBUG("Window size: %d %d", winSize.first, winSize.second);
    DEBUG("Pixel ratio: %.2lf", app->m_pixelRatio);
    // init m_layer
    auto layer = std::make_shared<VLayer>();
    if (layer)
    {
      LayerConfig cfg;
      cfg.drawableSize[0] = app->m_appConfig.windowSize[0];
      cfg.drawableSize[1] = app->m_appConfig.windowSize[1];
      cfg.dpi = app->Self()->getDPIScale();
      cfg.stencilBit = app->appConfig().videoConfig.stencilBit;
      cfg.multiSample = app->appConfig().videoConfig.multiSample;
      if (auto err = layer->init(cfg))
      {
        return AppError(AppError::Kind::RenderEngineError, "RenderEngineError");
      }
      app->m_layerAdapter = std::make_unique<EventDispatcherLayer>(std::move(layer));
    }
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
  bool onGlobalEvent(const UEvent& evt)
  {
    // handles App-wide events
    if (evt.type == VGG_QUIT)
    {
      m_shouldExit = true;
      return true;
    }
    return false;
  }

public: // public methods
  inline bool shouldExit()
  {
    return m_shouldExit;
  }

  bool sendEvent(const UEvent& e)
  {
    if (m_eventListener)
    {
      m_eventListener->onEvent(e, this);
    }
    if (m_layerAdapter)
    {
      m_layerAdapter->sendEvent(e, this);
    }
    return onGlobalEvent(e);
  }

  inline EventDispatcherLayer* layer()
  {
    ASSERT(m_layerAdapter.get());
    return m_layerAdapter.get();
  }

  void setEventListener(std::unique_ptr<EventListener> listener)
  {
    m_eventListener = std::move(listener);
  }

  const AppConfig& appConfig() const
  {
    return m_appConfig;
  }

  const std::string& appName() const
  {
    return m_appConfig.appName;
  }

  void poll()
  {
    ASSERT(m_inited);
    Self()->pollEvent();
  }

  void process()
  {
    ASSERT(m_inited);
    if (m_layerAdapter)
    {
      m_layerAdapter->beginFrame();
      m_layerAdapter->render();
      m_layerAdapter->endFrame();
    }
    Self()->swapBuffer();
  }

  int exec()
  {
    ASSERT(m_inited);
    while (!shouldExit())
    {
      poll();
      process();
    }
    return 0;
  }

public: // public static methods
  static T* getInstance(const AppConfig& cfg)
  {
    static_assert(std::is_base_of<App<T>, T>());
    static T s_app;
    // if already initialized, these init params are ignored
    if (!s_app.m_inited)
    {
      auto appResult = init(&s_app, cfg);
      if (appResult.has_value())
      {
        INFO("%s", appResult.value().text.c_str());
        return nullptr;
      }
    }
    return &s_app;
  }
}; // class App

}; // namespace VGGNew

#endif // __APP_HPP__
