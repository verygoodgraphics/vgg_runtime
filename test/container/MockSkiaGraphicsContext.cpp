/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MockSkiaGraphicsContext.hpp"

#include "Layer/Graphics/GraphicsContext.hpp"
#include "Utility/Log.hpp"

#include <gpu/GrDirectContext.h>
#include <gpu/mock/GrMockTypes.h>
#include <include/core/SkSurface.h>

ContextCreateProc MockSkiaGraphicsContext::contextCreateProc()
{
  return []()
  {
    GrMockOptions options;
    sk_sp<GrDirectContext> grContext = GrDirectContext::MakeMock(&options);
    return grContext;
  };
}

SurfaceCreateProc MockSkiaGraphicsContext::surfaceCreateProc()
{
  return [](GrDirectContext* context, int w, int h, const VGG::layer::ContextConfig& cfg)
  {
    DEBUG("make skia surface");
    return SkSurfaces::Null(w, h);
  };
}