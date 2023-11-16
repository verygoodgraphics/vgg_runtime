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
#pragma once
#include "GraphicsSkia.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"

namespace VGG::layer
{

// TODO:: refactor
class SkiaGraphicsContext : public layer::GraphicsContext
{
public:
  virtual SurfaceCreateProc surfaceCreateProc() = 0;
  virtual ContextCreateProc contextCreateProc() = 0;
  virtual void onInitProperties(ContextProperty& property) override
  {
    property.api = EGraphicsAPIBackend::API_METAL;
  }
};
} // namespace VGG::layer
