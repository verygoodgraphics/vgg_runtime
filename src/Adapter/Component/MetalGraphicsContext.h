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

#include "Layer/Graphics/ContextSkBase.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"

#include "VGG/Component/MetalComponent.hpp"

namespace VGG
{
class MetalGraphicsContextImpl;

class MetalGraphicsContext : public VGG::layer::SkiaGraphicsContext
{
  MetalGraphicsContextImpl* m_impl;

public:
  MetalGraphicsContext(MetalComponent::MTLHandle mtkView);

  int width();
  int height();

  virtual bool              swap() override;
  virtual bool              makeCurrent() override;
  virtual void              shutdown() override;
  virtual void*             contextInfo() override;
  virtual bool              onInit() override;
  virtual void              onInitProperties(VGG::layer::ContextProperty& property) override;
  virtual SurfaceCreateProc surfaceCreateProc() override;
  virtual ContextCreateProc contextCreateProc() override;
};

} // namespace VGG