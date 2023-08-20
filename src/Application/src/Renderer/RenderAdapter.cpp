#include "interface/Renderer/RenderAdapter.h"
#include "Scene/VGGLayer.h"
#include "Scene/Scene.h"

namespace VGG
{

class LayerEventAdapter__pImpl
{
public:
  VGG_DECL_API(LayerEventAdapter)
  std::vector<std::shared_ptr<EventListener>> listeners;
  std::queue<UEvent> msgQueue;
  std::shared_ptr<VLayer> layer;
  LayerEventAdapter__pImpl(LayerEventAdapter* api, std::shared_ptr<VLayer> layer)
    : q_ptr(api)
    , layer(std::move(layer))
  {
  }

  void dispatchEvent(UEvent e, void* userData)
  {
    for (const auto& l : listeners)
    {
      l->dispatchEvent(e, userData);
    }
    q_ptr->onEvent(e);
  }
};

LayerEventAdapter::LayerEventAdapter(std::shared_ptr<VLayer> layer)
  : d_ptr(new LayerEventAdapter__pImpl(this, std::move(layer)))
{
}
void LayerEventAdapter::postEvent(UEvent e)
{
  VGG_IMPL(LayerEventAdapter)
  _->msgQueue.push(e);
}
void LayerEventAdapter::sendEvent(UEvent e)
{
  VGG_IMPL(LayerEventAdapter)
  _->dispatchEvent(e, this);
}
void LayerEventAdapter::addSceneListener(std::shared_ptr<SceneEventListener> listener)
{
  VGG_IMPL(LayerEventAdapter)
  _->listeners.push_back(listener);
  _->layer->addRenderItem(std::move(listener));
}

void LayerEventAdapter::beginFrame()
{
  VGG_IMPL(LayerEventAdapter);
  d_ptr->layer->beginFrame();
  while (_->msgQueue.empty() == false)
  {
    const auto& e = _->msgQueue.front();
    _->dispatchEvent(e, this);
    _->msgQueue.pop();
  }
}
void LayerEventAdapter::render()
{
  d_ptr->layer->render();
}
void LayerEventAdapter::endFrame()
{
  d_ptr->layer->endFrame();
}

bool LayerEventAdapter::onEvent(UEvent e)
{
  VGG_IMPL(LayerEventAdapter);
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

LayerEventAdapter::~LayerEventAdapter()
{
}

} // namespace VGG
