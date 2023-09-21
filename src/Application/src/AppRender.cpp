
#include "Application/AppRender.h"
#include "AppRenderable.h"
#include "AppScene.h"
#include "Event/Event.h"
#include "Scene/VGGLayer.h"
#include "Application/interface/Event/EventListener.h"
#include "Utility/interface/CappingProfiler.hpp"

namespace VGG::app
{

class AppRender__pImpl
{
public:
  VGG_DECL_API(AppRender)
  std::vector<std::shared_ptr<AppScene>> listeners;
  std::vector<std::shared_ptr<AppRenderable>> renderables;
  std::queue<std::pair<UEvent, void*>> msgQueue;
  AppRender__pImpl(AppRender* api)
    : q_ptr(api)
  {
  }

  void dispatchEvent(UEvent e, void* userData)
  {
    q_ptr->onEvent(e);
    for (const auto& l : listeners)
    {
      l->onEvent(e, userData);
    }
    for (const auto& l : renderables)
    {
      l->onEvent(e, userData);
    }
  }

  ~AppRender__pImpl()
  {
  }
};

AppRender::AppRender()
  : d_ptr(new AppRender__pImpl(this))
{
}

void AppRender::postEvent(UEvent e, void* userData)
{
  VGG_IMPL(AppRender)
  _->msgQueue.emplace(e, userData);
}
void AppRender::sendEvent(UEvent e, void* userData)
{
  VGG_IMPL(AppRender)
  _->dispatchEvent(e, userData);
}
void AppRender::addAppScene(std::shared_ptr<AppScene> listener)
{
  VGG_IMPL(AppRender)
  _->listeners.push_back(listener);
  addScene(std::move(listener));
}

void AppRender::addAppRenderable(std::shared_ptr<AppRenderable> listener)
{
  VGG_IMPL(AppRender)
  _->renderables.push_back(listener);
  addRenderItem(std::move(listener));
}

bool AppRender::beginFrame(int fps)
{
  VGG_IMPL(AppRender);
  while (_->msgQueue.empty() == false)
  {
    auto& [e, u] = _->msgQueue.front();
    _->dispatchEvent(e, u);
    _->msgQueue.pop();
  }

  auto profiler = CappingProfiler::getInstance();
  if (fps > 0)
  {
    if (!(profiler->enoughFrameDuration(fps)))
    {
      return false;
    }
  }
  profiler->markFrame();
  VLayer::beginFrame();
  return true;
}
void AppRender::render()
{
  VLayer::render();
}
void AppRender::endFrame()
{
  VLayer::endFrame();
}

bool AppRender::onEvent(UEvent e)
{
  VGG_IMPL(AppRender);
  auto type = e.type;
  if (type == VGG_MOUSEMOTION)
  {
    if (enableDrawPosition())
    {
      drawPosition(e.motion.windowX, e.motion.windowY);
    }
  }
  if (type == EEventType::VGG_APP_INIT)
  {
    // for GUI front-end, an init event is necessary to trigger resize to create surface.
    DEBUG("Layer Initiailization: [%d, %d]", e.init.drawableWidth, e.init.drawableHeight);
    resize(e.init.drawableWidth, e.init.drawableHeight);
  }
  return false;
}

AppRender::~AppRender()
{
  shutdown();
}

} // namespace VGG::app
