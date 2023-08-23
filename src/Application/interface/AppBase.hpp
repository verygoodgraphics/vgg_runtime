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

#include <memory>
#include <optional>
#include <exception>
#include <Scene/GraphicsContext.h>
#include <Scene/VGGLayer.h>
#include <Application/include/Application/AppRender.h>
#include <Application/interface/Event/EventListener.h>
#include <Utility/interface/Log.h>

using namespace VGG;
namespace VGG::app
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
 * float resolutionScale()
 *	return current dpi
 *	*/
HAS_MEMBER_FUNCTION_DEF(resolutionScale)
/*
 * void pollEvent()
 * Polls the backends events
 * */
HAS_MEMBER_FUNCTION_DEF(pollEvent)

#undef HAS_MEMBER_FUNCTION_DEF

struct AppError
{
  enum class EKind
  {
    TextureSizeOutOfRangeError,
    EGLNoDisplayError,
    EGLGetAttribError,
    MakeCurrentContextError,
    HasInitError,
    UnknownError,
    RenderEngineError,
  };
  EKind kind;
  std::string text;
  AppError(EKind k, const std::string& error)
    : kind(k)
    , text(error)
  {
  }
};

struct AppConfig
{
  layer::ContextConfig graphicsContextConfig;
  std::string appName;
  std::unique_ptr<EventListener> eventListener;
  int argc;
  char** argv;
};

template<typename T>
class AppBase : public layer::GraphicsContext
{
  // NOLINTBEGIN
  inline T* Self()
  {
    static_assert(std::is_base_of<AppBase, T>::value);
    static_assert(has_member_pollEvent<T>::value);
    static_assert(has_member_resolutionScale<T>::value);
    return static_cast<T*>(this);
  }
  // NOLINTEND

protected:
  std::shared_ptr<AppRender> m_appRender;
  std::unique_ptr<EventListener> m_eventListener;
  AppConfig m_appConfig;
  bool m_shouldExit;
  bool m_init{ false };

private:
  std::optional<AppError> initInternal(AppConfig cfg)
  {
    m_appConfig = std::move(cfg);
    m_appRender = std::make_shared<AppRender>();
    m_eventListener = std::move(m_appConfig.eventListener);
    if (m_appRender)
    {
      m_appConfig.graphicsContextConfig.resolutionScale = Self()->resolutionScale();
      init(m_appConfig.graphicsContextConfig);
      if (auto err = m_appRender->init(this))
      {
        return AppError(AppError::EKind::RenderEngineError, "RenderEngineError");
      }
    }
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

  AppBase()
    : m_shouldExit(false)
    , m_init(false)
  {
  }

  bool hasInit() const
  {
    return m_init;
  }

  inline static T* s_instance{ nullptr };

public: // public methods
  inline bool shouldExit()
  {
    return m_shouldExit;
  }

  bool sendEvent(const UEvent& e)
  {
    auto handled = onGlobalEvent(e);
    if (!handled)
    {
      if (m_eventListener)
      {
        m_eventListener->onEvent(e, this);
      }
      if (m_appRender)
      {
        m_appRender->sendEvent(e, this);
      }
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

  static T& createInstance(AppConfig cfg)
  {
    static_assert(std::is_base_of<AppBase<T>, T>());
    static T s_app;
    if (!s_app.hasInit())
    {
      s_app.initInternal(std::move(cfg));
      s_instance = &s_app;
      UEvent evt;
      evt.type = VGG_APP_INIT;
      evt.init.argc = cfg.argc;
      evt.init.argv = cfg.argv;
      s_app.sendEvent(evt);
    }
    return s_app;
  }

  static T* app()
  {
    return s_instance;
  }

}; // class App
   //
}; // namespace VGG::app

#endif // __APP_HPP__
