#include "Layer/Core/EventManager.hpp"
#include "Layer/Core/RenderNode.hpp"

namespace VGG::layer
{

void EventManager::pollEvents()
{
  while (!sharedInstance().m_eventQueue.empty())
  {
    auto e = sharedInstance().m_eventQueue.front();
    sharedInstance().m_eventQueue.pop();
    switch (e.event)
    {
      case ENodeEvent::UPDATE:
        if (auto p = e.node.lock())
        {
          p->invalidate();
          p->m_state &= ~VNode::EState::UPDATE;
        }
        break;
    }
  }
}
} // namespace VGG::layer
