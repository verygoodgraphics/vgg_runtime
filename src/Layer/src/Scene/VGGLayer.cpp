#include "Scene/VGGLayer.h"
#include "Core/Node.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/Zoomer.h"
#include <optional>

#define GR_GL_LOG_CALLS 0
#define GR_GL_CHECK_ERROR 0
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkPicture.h>
#include <include/core/SkPictureRecorder.h>
#include <include/core/SkImage.h>
#include <include/core/SkSwizzle.h>
#include <include/core/SkTextBlob.h>
#include <include/core/SkTime.h>
#include <include/core/SkColorSpace.h>
#include <include/effects/SkDashPathEffect.h>
#include <src/gpu/ganesh/gl/GrGLDefines.h>
#include <src/gpu/ganesh/gl/GrGLUtil.h>
#include "include/gpu/gl/GrGLFunctions.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/core/SkStream.h"
#include "encode/SkPngEncoder.h"

#include "Scene/Scene.h"
#include "Common/Math.hpp"
#include "CappingProfiler.hpp"
#include "include/docs/SkPDFDocument.h"

#include <queue>

namespace VGG
{
struct SkiaState
{
private:
  std::unique_ptr<SkFILEWStream> m_stream;
  sk_sp<SkDocument> m_document;

public:
  sk_sp<const GrGLInterface> interface {
    nullptr
  };
  sk_sp<GrDirectContext> grContext{ nullptr };
  sk_sp<SkSurface> surface{ nullptr };

  inline SkCanvas* getPDFCanvas(const char* fileName, int width, int height)
  {
    m_stream = std::make_unique<SkFILEWStream>(fileName);
    if (!m_stream)
    {
      DEBUG("failed to create file stream: %s", fileName);
      return nullptr;
    }
    SkPDF::Metadata metadata;
    metadata.fTitle = "VGG";
    metadata.fCreator = "Example WritePDF() Function";
    metadata.fCreation = { 0, 2019, 1, 4, 31, 12, 34, 56 };
    metadata.fModified = { 0, 2019, 1, 4, 31, 12, 34, 56 };
    m_document = SkPDF::MakeDocument(m_stream.get(), metadata);

    if (!m_document)
    {
      DEBUG("failed to create document");
      return nullptr;
    }
    auto canvas = m_document->beginPage(width, height);
    return canvas;
  }

  inline void endPDFCanvas()
  {
    if (m_document)
    {
      m_document->close();
      m_document = nullptr;
    }
  }

  inline SkCanvas* getCanvas()
  {
    if (surface)
    {
      return surface->getCanvas();
    }
    return nullptr;
  }
};

class VLayer__pImpl
{
  VGG_DECL_API(VLayer);

public:
  static constexpr int N_MULTISAMPLE = 0;
  static constexpr int N_STENCILBITS = 8;

  Zoomer zoomer;
  SkiaState skiaState;
  Scene scene;
  int surfaceWidth;
  int surfaceHeight;
  SkCanvas* canvas{ nullptr };
  VLayer__pImpl(VLayer* api)
    : q_ptr(api)
  {
  }

  void updateSkiaEngineGL()
  {
    // Create Skia
    // get skia interface and make opengl context
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    if (!interface)
    {
      // return AppError(AppError::Kind::RenderEngineError, "Failed to make skia opengl interface");
    }
    sk_sp<GrDirectContext> grContext = GrDirectContext::MakeGL(interface);
    if (!grContext)
    {
      // return AppError(AppError::Kind::RenderEngineError, "Failed to make skia opengl context.");
    }
    skiaState.interface = interface;
    skiaState.grContext = grContext;
  }

  void resizeSkiaSurfaceGL(int w, int h)
  {
    skiaState.surface = createSkiaSurfaceGL(w, h);
    surfaceWidth = w;
    surfaceHeight = h;
  }

  sk_sp<SkSurface> createSkiaSurfaceGL(int w, int h)
  {
    ASSERT(skiaState.interface);
    ASSERT(skiaState.grContext);
    GrGLFramebufferInfo info;
    info.fFBOID = 0;
    // GR_GL_GetIntegerv(m_skiaState.interface.get(),
    //                   GR_GL_FRAMEBUFFER_BINDING,
    //                   (GrGLint*)&info.fFBOID);

    // color type and info format must be the followings for
    // both OpenGL and OpenGL ES, otherwise it will fail
    info.fFormat = GR_GL_RGBA8;
    GrBackendRenderTarget target(w, h, N_MULTISAMPLE, N_STENCILBITS, info);

    SkSurfaceProps props;
    return SkSurfaces::WrapBackendRenderTarget(skiaState.grContext.get(),
                                               target,
                                               kBottomLeft_GrSurfaceOrigin,
                                               SkColorType::kRGBA_8888_SkColorType,
                                               nullptr,
                                               &props);
  }

  void drawPositionInfo(SkCanvas* canvas, int curMouseX, int curMouseY)
  {
    SkPaint textPaint;
    static const char* s_infoFmt1 = "WindowPos: [%d, %d]";
    static const char* s_infoFmt2 = "ScenePos: [%d, %d]";

    float windowPos[2] = { (float)curMouseX, (float)curMouseY };
    float logixXY[2];
    zoomer.mapWindowPosToLogicalPosition(windowPos, 1.0, logixXY);
    char info[1024];
    sprintf(info, s_infoFmt1, (int)curMouseX, (int)curMouseY);
    textPaint.setColor(SK_ColorBLACK);
    SkFont font;
    font.setSize(32);
    canvas->drawSimpleText(info,
                           strlen(info),
                           SkTextEncoding::kUTF8,
                           curMouseX,
                           curMouseY,
                           font,
                           textPaint);

    sprintf(info, s_infoFmt2, (int)logixXY[0], (int)logixXY[1]);
    canvas->drawSimpleText(info,
                           strlen(info),
                           SkTextEncoding::kUTF8,
                           curMouseX,
                           curMouseY + 40,
                           font,
                           textPaint);
  }
};

std::optional<ELayerError> VLayer::init(const LayerConfig& cfg)
{
  return std::nullopt;
}
void VLayer::beginFrame()
{
}
void VLayer::render()
{
  VGG_IMPL(VLayer)
}
void VLayer::endFrame()
{
}
void VLayer::shutdown()
{
}
} // namespace VGG
  //
std::shared_ptr<GraphicsEventListener> makeDefaultEventListner();
