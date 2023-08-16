#include "Scene/GraphicsLayer.h"
#include "Application/include/Event/Event.h"
#include "Scene/Zoomer.h"
#include <core/SkCanvas.h>

namespace VGG
{

class GraphicsEventListener__pImpl
{
  VGG_DECL_API(GraphicsEventListener)
public:
  Zoomer zoomer;
  GraphicsEventListener__pImpl(GraphicsEventListener* api)
    : q_ptr(api)
  {
  }
  void onZoomIn(float ratio, SkCanvas* canvas)
  {
  }
  void onZoomOut(float ratio, SkCanvas* canvas)
  {
  }
  void onTranslate(float x, float y, SkCanvas* canvas)
  {
  }
};

GraphicsEventListener::GraphicsEventListener()
  : d_ptr(new GraphicsEventListener__pImpl(this))
{
}

void GraphicsEventListener::dispatchEvent(UEvent e, void* userData)
{
  auto canvas = reinterpret_cast<SkCanvas*>(userData);
}

std::shared_ptr<GraphicsEventListener> makeDefaultEventListner()
{
  return std::make_shared<GraphicsEventListener>();
}
}; // namespace VGG
