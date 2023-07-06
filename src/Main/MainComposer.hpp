#pragma once

#include "Controller/Controller.hpp"
#include "RunLoop.hpp"
#include "Model/VggWork.hpp"
#include "PlatformAdapter/Browser/Composer/BrowserComposer.hpp"
#include "PlatformAdapter/Native/Composer/NativeComposer.hpp"
#include "Utils/Async.hpp"
#include "View/UIView.hpp"

#include <memory>

namespace VGG
{
class MainComposer
{
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Presenter> m_presenter;

  std::shared_ptr<RunLoop> m_run_loop;
  std::shared_ptr<Controller> m_controller;

  std::shared_ptr<PlatformComposer> m_platform_composer;

public:
  MainComposer()
    : m_view{ std::make_shared<UIView>() }
    , m_presenter{ std::make_shared<Presenter>() }
    , m_run_loop{ std::make_shared<RunLoop>() }
    , m_controller{ std::make_shared<Controller>(m_run_loop, m_presenter) }
#ifdef EMSCRIPTEN
    , m_platform_composer{ new BrowserComposer() }
#else
#ifdef NDEBUG
    , m_platform_composer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js") }
#else
    , m_platform_composer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js", false) }
#endif
#endif
  {
    m_presenter->setView(m_view);

    m_platform_composer->setup();

#ifdef EMSCRIPTEN
    AsyncWorkerFactory::setTaskWorkerFactory([run_loop = m_run_loop]()
                                             { return run_loop->thread(); });
#else
    AsyncWorkerFactory::setTaskWorkerFactory([]() { return rxcpp::observe_on_new_thread(); });
#endif
    AsyncWorkerFactory::setResultWorkerFactory([run_loop = m_run_loop]()
                                               { return run_loop->thread(); });
  }

  ~MainComposer()
  {
    m_platform_composer->teardown();
  }

  auto runLoop()
  {
    return m_run_loop;
  }

  auto controller()
  {
    return m_controller;
  }

  auto view()
  {
    return m_view;
  }

#ifdef EMSCRIPTEN
  static MainComposer& instance()
  {
    static MainComposer instance;
    return instance;
  }
#endif
};

} // namespace VGG