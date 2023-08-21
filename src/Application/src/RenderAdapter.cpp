
#include "Application/RenderAdapter.h"
#include "Scene/VGGLayer.h"
#include "Scene/Scene.h"

namespace VGG
{

class EventDispatcherLayer__pImpl
{
public:
  VGG_DECL_API(EventDispatcherLayer)
  std::vector<std::shared_ptr<EventListener>> listeners;
  std::queue<std::pair<UEvent, void*>> msgQueue;
  std::shared_ptr<VLayer> layer;
  EventDispatcherLayer__pImpl(EventDispatcherLayer* api, std::shared_ptr<VLayer> layer)
    : q_ptr(api)
    , layer(std::move(layer))
  {
  }

  void dispatchEvent(UEvent e, void* userData)
  {
    for (const auto& l : listeners)
    {
      l->onEvent(e, userData);
    }
    q_ptr->onEvent(e);
  }
};

EventDispatcherLayer::EventDispatcherLayer(std::shared_ptr<VLayer> layer)
  : d_ptr(new EventDispatcherLayer__pImpl(this, std::move(layer)))
{
}
void EventDispatcherLayer::postEvent(UEvent e, void* userData)
{
  VGG_IMPL(EventDispatcherLayer)
  _->msgQueue.emplace(e, userData);
}
void EventDispatcherLayer::sendEvent(UEvent e, void* userData)
{
  VGG_IMPL(EventDispatcherLayer)
  _->dispatchEvent(e, userData);
}
void EventDispatcherLayer::addSceneListener(std::shared_ptr<EventListenerScene> listener)
{
  VGG_IMPL(EventDispatcherLayer)
  _->listeners.push_back(listener);
  _->layer->addRenderItem(std::move(listener));
}

void EventDispatcherLayer::beginFrame()
{
  VGG_IMPL(EventDispatcherLayer);
  d_ptr->layer->beginFrame();
  while (_->msgQueue.empty() == false)
  {
    auto& [e, u] = _->msgQueue.front();
    _->dispatchEvent(e, u);
    _->msgQueue.pop();
  }
}
void EventDispatcherLayer::render()
{
  d_ptr->layer->render();
}
void EventDispatcherLayer::endFrame()
{
  d_ptr->layer->endFrame();
}

bool EventDispatcherLayer::onEvent(UEvent e)
{
  VGG_IMPL(EventDispatcherLayer);
  auto type = e.type;
  if (auto& window = e.window;
      type == VGG_WINDOWEVENT && window.event == VGG_WINDOWEVENT_SIZE_CHANGED)
  {
    int w = window.data1;
    int h = window.data2;
    _->layer->resize(w, h);
    return true;
  }
  return false;
}

EventDispatcherLayer::~EventDispatcherLayer()
{
}

} // namespace VGG
