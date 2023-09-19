#include "Scene/GraphicsContext.h"
#include "Scene/GraphicsLayer.h"

namespace VGG::layer
{

// bool GraphicsContext::resize(int w, int h)
// {
//   m_config.windowSize[0] = w;
//   m_config.windowSize[1] = h;
//   // send resize msg to mananaged layers
//   for (const auto& l : m_managedLayer)
//   {
//     if (auto p = l.lock(); p)
//     {
//       p->resize(w, h);
//     }
//   }
//   return ok;
// }

} // namespace VGG::layer
