#pragma once

#include "Controller.hpp"
#include "PlatformComposer.hpp"
#include "Presenter.hpp"
#include "RunLoop.hpp"
#include "UIView.hpp"
#include "Async.hpp"
#include "Domain/Daruma.hpp"

#include <memory>

namespace VGG
{
class MainComposer
{
  std::shared_ptr<UIView> m_view;
  std::shared_ptr<UIView> m_edit_view;
  std::shared_ptr<Presenter> m_presenter;

  std::shared_ptr<RunLoop> m_run_loop;
  std::shared_ptr<Controller> m_controller;

  std::shared_ptr<PlatformComposer> m_platform_composer;

public:
  MainComposer(PlatformComposer* platformComposer)
    : m_view{ std::make_shared<UIView>() }
    , m_presenter{ std::make_shared<Presenter>() }
    , m_run_loop{ std::make_shared<RunLoop>() }
    , m_controller{ std::make_shared<Controller>(m_run_loop, m_presenter) }
    , m_platform_composer{ platformComposer }
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

  void enableEdit()
  {
    m_edit_view.reset(new UIView);
    m_presenter->setEditView(m_edit_view);

    m_view->disableSelfZoom();
    // todo: set subview's frame rect
    m_view->addSubview(m_edit_view);
  }
};

} // namespace VGG
