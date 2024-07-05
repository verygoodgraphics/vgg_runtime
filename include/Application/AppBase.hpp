/*
 * Copyright 2021-2023 Chaoya Li <harry75369@gmail.com>
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
#pragma once
#include "Application/AppRender.hpp"
#include "Application/Event/EventListener.hpp"
#include "Utility/Log.hpp"

#include "Layer/Graphics/GraphicsContext.hpp"

#include <memory>
#include <optional>
#include <exception>

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
 * void pollEvent()
 * Polls the backends events
 * */
HAS_MEMBER_FUNCTION_DEF(pollEvent)

#undef HAS_MEMBER_FUNCTION_DEF

struct AppError
{
  enum class EKind
  {
    TEXTURE_SIZE_OUT_OF_RANGE_ERROR,
    EGL_NO_DISPLAY_ERROR,
    EGL_GET_ATTRIB_ERROR,
    MAKE_CURRENT_CONTEXT_ERROR,
    HAS_INIT_ERROR,
    UNKNOWN_ERROR,
    RENDER_ENGINE_ERROR,
  };
  EKind       kind;
  std::string text;
  AppError(EKind k, const std::string& error)
    : kind(k)
    , text(error)
  {
  }
};

struct AppConfig
{
  layer::ContextConfig           graphicsContextConfig;
  std::string                    appName;
  std::unique_ptr<EventListener> eventListener;
  int                            windowSize[2];
  int                            renderFPSLimit{ 60 };
  int                            argc;
  char**                         argv;
};

template<typename T>
class AppBase : public layer::GraphicsContext
{
  // NOLINTBEGIN
  inline T* Self()
  {
    static_assert(std::is_base_of<AppBase, T>::value);
    static_assert(has_member_pollEvent<T>::value);
    return static_cast<T*>(this);
  }
  // NOLINTEND

protected:
  std::shared_ptr<AppRender>     m_appRender;
  std::unique_ptr<EventListener> m_eventListener;
  AppConfig                      m_appConfig;
  bool                           m_shouldExit;
  bool                           m_init{ false };

private:
  std::optional<AppError> initInternal(AppConfig cfg)
  {
    m_appConfig = std::move(cfg);
    m_appRender = std::make_shared<AppRender>();
    m_eventListener = std::move(m_appConfig.eventListener);
    if (m_appRender)
    {
      init(m_appConfig.graphicsContextConfig);
      if (auto err = m_appRender->init(this))
      {
        return AppError(AppError::EKind::RENDER_ENGINE_ERROR, "RENDER_ENGINE_ERROR");
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
        evt.type == VGG_WINDOWEVENT &&
        (window.event == VGG_WINDOWEVENT_RESIZED || window.event == VGG_WINDOWEVENT_SIZE_CHANGED))
    {
      updateDpi(Self()->resolutionScale());
      if (m_appRender)
      {
        int drawableWidth = window.drawableWidth;
        int drawableHeight = window.drawableHeight;
        m_appRender->resize(drawableWidth, drawableHeight);
      }
    }

    return false; // continue to dispatch event
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
    ASSERT(m_appRender);
    if (m_appRender->beginFrame(appConfig().renderFPSLimit))
    {
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
      evt.init.windowWidth = cfg.windowSize[0];
      evt.init.windowHeight = cfg.windowSize[1];
      const auto p = s_app.property();
      evt.init.drawableWidth = cfg.windowSize[0] * p.dpiScaling;
      evt.init.drawableHeight = cfg.windowSize[1] * p.dpiScaling;
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
