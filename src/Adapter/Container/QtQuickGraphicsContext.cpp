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

#include "QtQuickGraphicsContext.hpp"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

using namespace VGG;

QtQuickGraphicsContext::QtQuickGraphicsContext(unsigned int fboID, double dpi)
  : m_fboID(fboID)
  , m_dpi(dpi)
  , m_context(GrDirectContext::MakeGL())
{
}

bool QtQuickGraphicsContext::swap()
{
  return true;
}

bool QtQuickGraphicsContext::makeCurrent()
{
  return true;
}

void QtQuickGraphicsContext::shutdown()
{
}

void* QtQuickGraphicsContext::contextInfo()
{
  return nullptr;
}

bool QtQuickGraphicsContext::onInit()
{
  return true;
}

void QtQuickGraphicsContext::onInitProperties(layer::ContextProperty& property)
{
  property.dpiScaling = static_cast<float>(m_dpi);
  property.api = VGG::layer::EGraphicsAPIBackend::API_CUSTOM;
}

SurfaceCreateProc QtQuickGraphicsContext::surfaceCreateProc()
{
  auto fboID = m_fboID;
  return [fboID](GrRecordingContext* context, int w, int h, const VGG::layer::ContextConfig& cfg)
  {
    GrGLFramebufferInfo info;
    info.fFBOID = fboID;
    info.fFormat = GR_GL_RGBA8;
    GrBackendRenderTarget target(w, h, cfg.multiSample, cfg.stencilBit, info);
    SkSurfaceProps        props;
    return SkSurfaces::WrapBackendRenderTarget(
      context,
      target,
      kTopLeft_GrSurfaceOrigin,
      SkColorType::kRGBA_8888_SkColorType,
      nullptr,
      &props);
  };
}

ContextCreateProc QtQuickGraphicsContext::contextCreateProc()
{
  auto tmp = m_context;
  return [tmp]() { return tmp; };
}