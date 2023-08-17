#include "Scene/GraphicsLayer.h"
#include "Application/include/Event/Event.h"
#include "Scene/EventListener.h"
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
  std::queue<UEvent> msgQueue;
  Graphics__pImpl(Graphics* api)
    : q_ptr(api)
  {
  }
};

Graphics::Graphics()
  : d_ptr(new Graphics__pImpl(this))
{
}

void Graphics::pushEvent(UEvent e)
{
  VGG_IMPL(Graphics)
  _->msgQueue.push(e);
}

void Graphics::addEventListener(std::shared_ptr<EventListener> listener)
{
  VGG_IMPL(Graphics)
  _->listeners.push_back(std::move(listener));
}
void Graphics::dispatchEvent()
{
  VGG_IMPL(Graphics)
  while (_->msgQueue.empty() == false)
  {
    const auto& e = _->msgQueue.front();
    for (const auto& l : _->listeners)
    {
      l->dispatchEvent(e);
    }
    _->msgQueue.pop();
  }
}

Graphics::~Graphics()
{
}

}; // namespace VGG
