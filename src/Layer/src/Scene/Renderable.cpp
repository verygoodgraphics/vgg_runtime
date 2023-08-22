#include "Scene/Renderable.h"
#include "core/SkCanvas.h"

namespace VGG::layer
{
void Renderable::onRender(SkCanvas* canvas)
{
  if (m_viewport)
  {
    canvas->save();
    canvas->translate(m_viewport->position[0], m_viewport->position[1]);
    SkRect rect{ 0, 0, (float)m_viewport->extent[0], (float)m_viewport->extent[1] };
    canvas->clipRect(rect);
    onRenderImpl(canvas);
    canvas->restore();
  }
  else
  {
    onRenderImpl(canvas);
  }
}
}; // namespace VGG::layer
