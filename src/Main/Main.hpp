#pragma once

#include "Controller/Controller.hpp"
#include "RunLoop.hpp"
#include "Model/VggWork.hpp"
#include "PlatformAdapter/Native/Composer/MainComposer.hpp"
#include "View/UIView.hpp"

#include <memory>

namespace VGG
{
class Main
{
  std::shared_ptr<RunLoop> m_run_loop;
  std::shared_ptr<Controller> m_controller;
  std::shared_ptr<VggWork> m_model;
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<Presenter> m_presenter;
#ifdef NDEBUG
  MainComposer m_main_composer{ true };
#else
  MainComposer m_main_composer{ false }; // false: enable js throw error to abort subprocess
#endif

public:
  Main()
  {
    m_view = std::make_shared<UIView>();
    m_presenter = std::make_shared<Presenter>();
    m_presenter->setView(m_view);

    m_run_loop = std::make_shared<RunLoop>();
    m_controller = std::make_shared<Controller>(m_run_loop, m_presenter);

    // todo: browser env

    // local di container
    m_main_composer.setup(
      "/Users/houguanhua/code/vgg/vgg_runtime/test/testDataDir/fake-sdk/vgg-sdk.esm.mjs");
    // todo: remote di container
    // m_main_composer.setup("./asset/vgg-sdk.esm.mjs");
  }

  ~Main()
  {
    m_main_composer.teardown();
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
};

} // namespace VGG