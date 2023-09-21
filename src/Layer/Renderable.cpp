#include "VGG/Layer/Renderable.hpp"

#include <core/SkCanvas.h>

namespace VGG::layer
{
void Renderable::render(SkCanvas* canvas)
{
  if (m_visible)
  {
    if (m_viewport)
    {
      canvas->save();
      canvas->translate(m_viewport->position[0], m_viewport->position[1]);
      SkRect rect{ 0, 0, (float)m_viewport->extent[0], (float)m_viewport->extent[1] };
      canvas->clipRect(rect);
      onRender(canvas);
      canvas->restore();
    }
    else
    {
      onRender(canvas);
    }
  }
}
}; // namespace VGG::layer
