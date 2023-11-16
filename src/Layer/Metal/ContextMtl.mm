#include "Layer/Graphics/Metal/ContextMtl.hpp"
#include "Layer/Graphics/GraphicsSkia.hpp"

#include <include/core/SkSurface.h>
#include <include/gpu/GrDirectContext.h>

#ifdef VGG_USE_METAL
namespace VGG::layer
{
SurfaceCreateProc MtlGraphicsContext::surfaceCreateProc()
{
  return [this](GrDirectContext* context, int w, int h, const ContextConfig& cfg)
  { return nullptr; };
}

ContextCreateProc MtlGraphicsContext::contextCreateProc()
{
  return [this]() { return nullptr; };
}
} // namespace VGG::layer
#endif
