/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#include "QtGraphicsContext.hpp"

// skia
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/ports/SkCFObject.h"

#include "Utility/Log.hpp"

namespace VGG
{
// impl ------------------------------------------------------------------------
class QtGraphicsContextImpl
{
  QtGraphicsContext*     m_api;
  float                  m_devicePixelRatio;
  sk_sp<GrDirectContext> m_context;

public:
  QtGraphicsContextImpl(QtGraphicsContext* api)
    : m_api(api)
  {
    m_context = GrDirectContext::MakeGL();
  }

  bool swap()
  {
    return true;
  }

  SurfaceCreateProc surfaceCreateProc()
  {
    return [](GrRecordingContext* context, int w, int h, const VGG::layer::ContextConfig& cfg)
    {
      auto info = SkImageInfo::MakeN32Premul(w, h);
      auto surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, info);
      return surface;
    };
  }

  ContextCreateProc contextCreateProc()
  {
    return [tmpContext = m_context]() { return tmpContext; };
  }

  void onInitProperties(VGG::layer::ContextProperty& property)
  {
    property.dpiScaling = m_devicePixelRatio;
    property.api = VGG::layer::EGraphicsAPIBackend::API_CUSTOM;
  }

  void init(float devicePixelRatio)
  {
    m_devicePixelRatio = devicePixelRatio;
  }

private:
};

// api ------------------------------------------------------------------------
QtGraphicsContext::QtGraphicsContext()
  : m_impl(new QtGraphicsContextImpl(this))
{
}

void QtGraphicsContext::init(float devicePixelRatio)
{
  m_impl->init(devicePixelRatio);
}

bool QtGraphicsContext::swap()
{
  return m_impl->swap();
}

bool QtGraphicsContext::makeCurrent()
{
  return true;
}

void QtGraphicsContext::shutdown()
{
  return;
}

void* QtGraphicsContext::contextInfo()
{
  return nullptr;
}

bool QtGraphicsContext::onInit()
{
  return true;
}

SurfaceCreateProc QtGraphicsContext::surfaceCreateProc()
{
  return m_impl->surfaceCreateProc();
}

ContextCreateProc QtGraphicsContext::contextCreateProc()
{
  return m_impl->contextCreateProc();
}

void QtGraphicsContext::onInitProperties(VGG::layer::ContextProperty& property)
{
  m_impl->onInitProperties(property);
}

} // namespace VGG
