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

#include <SDL2/SDL_pixels.h>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <queue>
#include <fstream>
#include <memory>
#include <optional>
#include <regex>
#include <any>
#include <stdexcept>
#include <exception>
#include "Scene/GraphicsContext.h"
#include "Scene/VGGLayer.h"
#include "Application/AppRender.h"
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
 * float getDPIScale()
 *	return current dpi
 *	*/
HAS_MEMBER_FUNCTION_DEF(getDPIScale)
/*
 * std::any setProperty(const std::string &, std::any value):
 *	It should return the properties about the T
 *	"window_size", "app_size", "viewport_size"
 *
 * */

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
  AppConfig() = default;
};

template<typename T>
class App : public layer::GraphicsContext
{
  // NOLINTBEGIN
  inline T* Self()
  {
    static_assert(std::is_base_of<App, T>::value);
    static_assert(has_member_onInit<T>::value);
    static_assert(has_member_pollEvent<T>::value);
    return static_cast<T*>(this);
  }
  // NOLINTEND

protected:
  std::unique_ptr<AppRender> m_appRender;
  std::unique_ptr<EventListener> m_eventListener;
  AppConfig m_appConfig;
  bool m_shouldExit;
  double m_pixelRatio{ 1.0 };
  bool m_init{ false };

private:
  std::optional<AppError> initInternal(const AppConfig& cfg)
  {
    m_appConfig = cfg;
    int w = m_appConfig.windowSize[0];
    int h = m_appConfig.windowSize[1];
    // auto appResult = Self()->initContext(w * m_pixelRatio, h * m_pixelRatio);
    // if (appResult.has_value())
    //   return appResult;
    //
    // appResult = Self()->makeContextCurrent();
    // if (appResult.has_value())
    //   return appResult;

    // get necessary property about window and DPI
    // auto drawSize = std::any_cast<std::pair<int, int>>(Self()->getProperty("viewport_size"));
    // auto winSize = std::any_cast<std::pair<int, int>>(Self()->getProperty("window_size"));

#ifdef EMSCRIPTEN
    app->m_pixelRatio = (double)drawSize.first / winSize.first;
#endif

    // DEBUG("Drawable size: %d %d", drawSize.first, drawSize.second);
    // DEBUG("Window size: %d %d", winSize.first, winSize.second);
    // DEBUG("Pixel ratio: %.2lf", m_pixelRatio);
    // init m_layer
    m_appRender = std::make_unique<AppRender>();
    if (m_appRender)
    {
      layer::ContextConfig cfg;
      cfg.drawableSize[0] = m_appConfig.windowSize[0];
      cfg.drawableSize[1] = m_appConfig.windowSize[1];
      cfg.dpi = Self()->getDPIScale();
      cfg.stencilBit = appConfig().videoConfig.stencilBit;
      cfg.multiSample = appConfig().videoConfig.multiSample;
      initGraphicsContext(cfg);
      // cfg.drawableSize[0] = m_appConfig.windowSize[0];
      // cfg.drawableSize[1] = m_appConfig.windowSize[1];
      // cfg.dpi = Self()->getDPIScale();
      // cfg.context = this;
      if (auto err = m_appRender->init(this))
      {
        return AppError(AppError::Kind::RenderEngineError, "RenderEngineError");
      }
    }
    // extra initialization
    Self()->onInit();
    return std::nullopt;
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
    if (auto& window = evt.window;
        evt.type == VGG_WINDOWEVENT && window.event == VGG_WINDOWEVENT_SIZE_CHANGED)
    {
      int w = window.data1;
      int h = window.data2;
      resize(w, h);
      return true;
    }
    return false;
  }

  App()
    : m_shouldExit(false)
    , m_pixelRatio(1.0)
    , m_init(false)
  {
  }

  bool hasInit() const
  {
    return m_init;
  }

public: // public methods
  inline bool shouldExit()
  {
    return m_shouldExit;
  }

  bool sendEvent(const UEvent& e)
  {
    onGlobalEvent(e);
    if (m_eventListener)
    {
      m_eventListener->onEvent(e, this);
    }
    if (m_appRender)
    {
      m_appRender->sendEvent(e, this);
    }
    return true;
  }

  inline AppRender* layer()
  {
    ASSERT(m_appRender.get());
    return m_appRender.get();
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
    Self()->pollEvent();
  }

  void process()
  {
    if (m_appRender)
    {
      m_appRender->beginFrame();
      m_appRender->render();
      m_appRender->endFrame();
    }
  }

  int exec()
  {
    while (!shouldExit())
    {
      poll();
      process();
    }
    return 0;
  }

public: // public static methods
  static T& createInstance(const AppConfig& cfg = AppConfig())
  {
    static_assert(std::is_base_of<App<T>, T>());
    static T s_app;
    if (!s_app.hasInit())
    {
      s_app.initInternal(cfg);
    }
    return s_app;
  }

}; // class App
   //
}; // namespace VGGNew

#endif // __APP_HPP__
