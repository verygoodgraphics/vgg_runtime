#include "Application/MainComposer.hpp"

#include "Editor.hpp"

using namespace VGG;

MainComposer::MainComposer(PlatformComposer* platformComposer, std::shared_ptr<Mouse> mouse)
  : m_view{ new UIView }
  , m_presenter{ new Presenter }
  , m_editor{ new Editor{ m_view, mouse } }
  , m_runLoop{ new RunLoop }
  , m_controller{ new Controller{ m_runLoop, m_presenter, m_editor } }
  , m_platformComposer{ platformComposer }
{
  m_presenter->setView(m_view);

  m_platformComposer->setup();

#ifdef EMSCRIPTEN
  AsyncWorkerFactory::setTaskWorkerFactory([runLoop = m_runLoop]() { return runLoop->thread(); });
#else
  AsyncWorkerFactory::setTaskWorkerFactory([]() { return rxcpp::observe_on_new_thread(); });
#endif
  AsyncWorkerFactory::setResultWorkerFactory([runLoop = m_runLoop]() { return runLoop->thread(); });
}