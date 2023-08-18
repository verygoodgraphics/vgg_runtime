#include "Scene/GraphicsLayer.h"
#include "Application/include/Event/Event.h"
#include "Scene/Zoomer.h"
#include <core/SkCanvas.h>

#include <queue>

namespace VGG
{

class Graphics__pImpl
{
  VGG_DECL_API(Graphics)
public:
  std::vector<std::shared_ptr<EventListener>> listeners;
  std::vector<std::shared_ptr<RenderEventListener>> renderable;
  std::queue<UEvent> msgQueue;
  Zoomer globalZoomer;
  Graphics__pImpl(Graphics* api)
    : q_ptr(api)
  {
  }

  void dispatchEvent(UEvent e, void* userData)
  {
    q_ptr->onEvent(e);
    for (const auto& l : listeners)
    {
      l->dispatchEvent(e, userData);
    }
    for (const auto& l : renderable)
    {
      l->dispatchEvent(e, userData);
    }
  }
};

Graphics::Graphics()
  : d_ptr(new Graphics__pImpl(this))
{
}

void Graphics::postEvent(UEvent e)
{
  VGG_IMPL(Graphics)
  _->msgQueue.push(e);
}

void Graphics::sendEvent(UEvent e)
{
  VGG_IMPL(Graphics)
  _->dispatchEvent(e, this);
}

void Graphics::addEventListener(std::shared_ptr<EventListener> listener)
{
  VGG_IMPL(Graphics)
  _->listeners.push_back(std::move(listener));
}

void Graphics::addRenderListener(std::shared_ptr<RenderEventListener> listener)
{
  VGG_IMPL(Graphics)
  _->renderable.push_back(std::move(listener));
}

void Graphics::beginFrame()
{
  VGG_IMPL(Graphics)
  while (_->msgQueue.empty() == false)
  {
    const auto& e = _->msgQueue.front();
    _->dispatchEvent(e, this);
    _->msgQueue.pop();
  }
}

void Graphics::sendRenderEvent(VPaintEvent e)
{
  VGG_IMPL(Graphics)
  for (const auto& l : _->renderable)
  {
    l->onPaintEvent(e);
  }
}

Graphics::~Graphics()
{
}

}; // namespace VGG
