#include <Application/MainComposer.hpp>

#include "Editor.hpp"

using namespace VGG;

MainComposer::MainComposer(PlatformComposer* platformComposer)
  : m_view{ new UIView }
  , m_presenter{ new Presenter }
  , m_editor{ new Editor{ m_view } }
  , m_runLoop{ new RunLoop }
  , m_controller{ new Controller{ m_runLoop, m_presenter, m_editor } }
  , m_platform_composer{ platformComposer }
{
  m_presenter->setView(m_view);

  m_platform_composer->setup();

#ifdef EMSCRIPTEN
  AsyncWorkerFactory::setTaskWorkerFactory([run_loop = m_runLoop]() { return run_loop->thread(); });
#else
  AsyncWorkerFactory::setTaskWorkerFactory([]() { return rxcpp::observe_on_new_thread(); });
#endif
  AsyncWorkerFactory::setResultWorkerFactory([run_loop = m_runLoop]()
                                             { return run_loop->thread(); });
}