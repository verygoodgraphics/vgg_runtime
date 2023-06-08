#pragma once

#include "Controller/Controller.hpp"
#include "RunLoop.hpp"
#include "Model/VggWork.hpp"
#include "PlatformAdapter/Browser/Composer/BrowserComposer.hpp"
#include "PlatformAdapter/Native/Composer/NativeComposer.hpp"
#include "View/UIView.hpp"

#include <memory>

namespace VGG
{
class MainComposer
{
  std::shared_ptr<RunLoop> m_run_loop;
  std::shared_ptr<Controller> m_controller;
  std::shared_ptr<VggWork> m_model;
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Presenter> m_presenter;
  std::shared_ptr<PlatformComposer> m_platform_composer;

public:
  MainComposer()
  {
    m_view = std::make_shared<UIView>();
    m_presenter = std::make_shared<Presenter>();
    m_presenter->setView(m_view);

    m_run_loop = std::make_shared<RunLoop>();
    m_controller = std::make_shared<Controller>(m_run_loop, m_presenter);

#ifdef EMSCRIPTEN
    m_platform_composer.reset(new BrowserComposer());
#else
#ifdef NDEBUG
    // remote di container
    m_platform_composer.reset(new NativeComposer("./asset/vgg-sdk.esm.mjs"));
#else
    // local di container
    m_platform_composer.reset(
      new NativeComposer("../test/testDataDir/fake-sdk/vgg-sdk.esm.mjs", false));
#endif
#endif
    m_platform_composer->setup();
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