#include "Scene/GraphicsContext.h"
#include "Scene/GraphicsLayer.h"

namespace VGG::layer
{

bool GraphicsContext::resize(int w, int h)
{
  auto ok = onResize(w, h);
  // send resize msg to mananaged layers
  for (const auto& l : m_managedLayer)
  {
    l->resize(w, h);
  }
  return ok;
}

} // namespace VGG::layer
