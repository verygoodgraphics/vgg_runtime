#include "Application/MainComposer.hpp"
#include "Async.hpp"

#include "Editor.hpp"
#include "Presenter.hpp"
#include "RunLoop.hpp"
#include "UIView.hpp"

using namespace VGG;

MainComposer::MainComposer(PlatformComposer* platformComposer, std::shared_ptr<Mouse> mouse)
  : m_view{ new UIView }
  , m_presenter{ new Presenter }
  , m_editor{ new Editor{ m_view, mouse } }
  , m_runLoop{ new RunLoop }
  , m_platformComposer{ platformComposer }
{
  m_presenter->setView(m_view);

  auto jsEngine = m_platformComposer->setup();
  m_controller.reset(new Controller{ m_runLoop, jsEngine, m_presenter, m_editor });

#ifdef EMSCRIPTEN
  AsyncWorkerFactory::setTaskWorkerFactory([runLoop = m_runLoop]() { return runLoop->thread(); });
#else
  AsyncWorkerFactory::setTaskWorkerFactory([]() { return rxcpp::observe_on_new_thread(); });
#endif
  AsyncWorkerFactory::setResultWorkerFactory([runLoop = m_runLoop]() { return runLoop->thread(); });
}

void MainComposer::enableEdit(int top, int right, int bottom, int left)
{
  m_editView.reset(new UIView);
  m_presenter->setEditView(m_editView);

  if (m_view)
  {
    m_view->becomeEditorWithSidebar(top, right, bottom, left);
    m_view->addSubview(m_editView);
  }
}