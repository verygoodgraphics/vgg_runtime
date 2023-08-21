
#include "Application/AppRender.h"
#include "AppScene.h"
#include "Scene/VGGLayer.h"
#include "Application/interface/Event/EventListener.h"

namespace VGG
{

class AppRender__pImpl
{
public:
  VGG_DECL_API(AppRender)
  std::vector<std::shared_ptr<AppScene>> listeners;
  std::queue<std::pair<UEvent, void*>> msgQueue;
  float mouseX, mouseY;
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

void AppRender::beginFrame()
{
  VGG_IMPL(AppRender);
  while (_->msgQueue.empty() == false)
  {
    auto& [e, u] = _->msgQueue.front();
    _->dispatchEvent(e, u);
    _->msgQueue.pop();
  }
  VLayer::beginFrame();
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
  if (auto& window = e.window;
      type == VGG_WINDOWEVENT && window.event == VGG_WINDOWEVENT_SIZE_CHANGED)
  {
    int w = window.data1;
    int h = window.data2;
    resize(w, h);
    return true;
  }
  if (type == VGG_MOUSEMOTION)
  {
    if (enableDrawPosition())
    {
      DebugInfo debugInfo;
      debugInfo.curX = e.motion.x;
      debugInfo.curY = e.motion.y;
      drawDebugInfo(debugInfo);
    }
  }
  return false;
}

AppRender::~AppRender()
{
  shutdown();
}

} // namespace VGG
