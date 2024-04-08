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

#include "MetalGraphicsContext.h"

// skia
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/SkSurfaceMetal.h"
#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "include/ports/SkCFObject.h"

// ios
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

#include "Utility/Log.hpp"

namespace {

sk_sp<SkSurface> SkMtkViewToSurface(MTKView* mtkView, GrRecordingContext* rContext)
{
  if (!rContext)
  {
    return nullptr;
  }
  if (MTLPixelFormatDepth32Float_Stencil8 != [mtkView depthStencilPixelFormat]
   || MTLPixelFormatRGBA8Unorm != [mtkView colorPixelFormat])
  {
    WARN("Inconsistent pixel format with mtkView! Expected MTLPixelFormatRGBA8Unorm and MTLPixelFormatDepth32Float_Stencil8");
  }

  const SkColorType colorType = kRGBA_8888_SkColorType;  // MTLPixelFormatRGBA8Unorm
  sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatRGBA8Unorm
  const GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
  const SkSurfaceProps surfaceProps;
  int sampleCount = (int)[mtkView sampleCount];

  return SkSurfaces::WrapMTKView(
    rContext,
    (__bridge GrMTLHandle)mtkView,
    origin,
    sampleCount,
    colorType,
    colorSpace,
    &surfaceProps);
}

}

namespace VGG
{
// impl ------------------------------------------------------------------------
class MetalGraphicsContextImpl
{
  MetalGraphicsContext*       m_api;
  MTKView*                    m_mtkView;
  sk_cfp<id<MTLDevice>>       m_device;
  sk_cfp<id<MTLCommandQueue>> m_queue;
  sk_sp<GrDirectContext>      m_Context;

public:
  MetalGraphicsContextImpl(MetalGraphicsContext* api, MetalContainer::MTLHandle mtkView)
    : m_api(api)
  {
    setView(mtkView);
  }

  int width()
  {
    return m_mtkView.frame.size.width;
  }

  int height()
  {
    return m_mtkView.frame.size.height;
  }

  bool swap()
  {
    id<MTLCommandBuffer> commandBuffer = [m_queue.get() commandBuffer];
    [commandBuffer presentDrawable:[m_mtkView currentDrawable]];
    [commandBuffer commit];
    return true;
  }

  SurfaceCreateProc surfaceCreateProc()
  {
    return [mtkView = m_mtkView](GrDirectContext* context, int w, int h, const VGG::layer::ContextConfig& cfg)
    {
      return SkMtkViewToSurface(mtkView, context);
    };
  }

  ContextCreateProc contextCreateProc()
  {
    return [tmpContext = m_Context]()
    {
      return tmpContext;
    };
  }

  void onInitProperties(VGG::layer::ContextProperty& property)
  {
    property.dpiScaling = m_mtkView.contentScaleFactor;
    property.api = VGG::layer::EGraphicsAPIBackend::API_CUSTOM;
  }

private:
  void setView(MetalContainer::MTLHandle view)
  {
    m_mtkView = (MTKView *)view;

    if(m_mtkView) {
      m_device.reset(m_mtkView.device);
      m_queue.reset([*m_device newCommandQueue]);

      GrMtlBackendContext backendContext = {};
      backendContext.fDevice.retain((__bridge GrMTLHandle)(m_device.get()));
      backendContext.fQueue.retain((__bridge GrMTLHandle)(m_queue.get()));
      m_Context = GrDirectContext::MakeMetal(backendContext, GrContextOptions());
    }
  }
};

// api ------------------------------------------------------------------------
MetalGraphicsContext::MetalGraphicsContext(MetalContainer::MTLHandle mtkView)
  : m_impl(new MetalGraphicsContextImpl(this, mtkView))
{
}

int MetalGraphicsContext::width()
{
  return m_impl->width();
}

int MetalGraphicsContext::height()
{
  return m_impl->height();
}

bool MetalGraphicsContext::swap()
{
  return m_impl->swap();
}


bool MetalGraphicsContext::makeCurrent()
{
  return true;
}

void MetalGraphicsContext::shutdown()
{
  return;
}

void* MetalGraphicsContext::contextInfo()
{
  return nullptr;
}

bool MetalGraphicsContext::onInit()
{
  return true;
}

SurfaceCreateProc MetalGraphicsContext::surfaceCreateProc()
{
  return m_impl->surfaceCreateProc();
}

ContextCreateProc MetalGraphicsContext::contextCreateProc()
{
  return m_impl->contextCreateProc();
}

void MetalGraphicsContext::onInitProperties(VGG::layer::ContextProperty& property)
{
  m_impl->onInitProperties(property);
}

}
