#include "Scene/VGGLayer.h"
#include "Core/Node.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/Renderer.h"
#include "Scene/Zoomer.h"
#include <optional>
#include <sstream>

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
    return surface->getCanvas();
  }
};

class VLayer__pImpl
{
  VGG_DECL_API(VLayer);

public:
  LayerConfig config;
  Zoomer zoomer;
  SkiaState skiaState;
  int surfaceWidth;
  int surfaceHeight;
  SkCanvas* canvas{ nullptr };
  std::vector<std::shared_ptr<Renderable>> items;
  std::vector<std::shared_ptr<Scene>> scenes;
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
    GrBackendRenderTarget target(w, h, config.stencilBit, config.multiSample, info);

    SkSurfaceProps props;
    return SkSurfaces::WrapBackendRenderTarget(skiaState.grContext.get(),
                                               target,
                                               kBottomLeft_GrSurfaceOrigin,
                                               SkColorType::kRGBA_8888_SkColorType,
                                               nullptr,
                                               &props);
  }

  std::string mapMousePositionToScene(const Zoomer* zoomer,
                                      const char* name,
                                      float scaleFactor,
                                      int curMouseX,
                                      int curMouseY)
  {
    std::stringstream ss;
    float windowPos[2] = { (float)curMouseX, (float)curMouseY };
    float logixXY[2];
    zoomer->mapWindowPosToLogicalPosition(windowPos, scaleFactor, logixXY);
    ss << name << ": (" << (int)logixXY[0] << ", " << (int)logixXY[1] << ")";
    std::string res{ std::istreambuf_iterator<char>{ ss }, {} };
    return res;
  }

  void drawTextAt(SkCanvas* canvas,
                  const std::vector<std::string>& strings,
                  int curMouseX,
                  int curMouseY)
  {
    SkPaint textPaint;
    textPaint.setColor(SK_ColorBLACK);
    SkFont font;
    constexpr int FONTSIZE = 20;
    font.setSize(FONTSIZE);
    int dy = 0;
    for (const auto& text : strings)
    {
      canvas->drawSimpleText(text.c_str(),
                             text.size(),
                             SkTextEncoding::kUTF8,
                             curMouseX,
                             curMouseY + dy,
                             font,
                             textPaint);
      dy += FONTSIZE;
    }
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
  VGG_IMPL(VLayer)
  _->config = cfg;
  _->updateSkiaEngineGL();
  resize(cfg.drawableSize[0], cfg.drawableSize[1]);
  return std::nullopt;
}
void VLayer::beginFrame()
{
}

void VLayer::render()
{
  VGG_IMPL(VLayer)
  auto canvas = _->skiaState.getCanvas();
  if (canvas)
  {
    canvas->save();
    canvas->clear(SK_ColorWHITE);
    float sx = 1.f, sy = 1.f;
    canvas->scale(sx, sy);
    for (auto& scene : _->scenes)
    {
      scene->onRender(canvas);
    }
    for (auto& item : _->items)
    {
      item->onRender(canvas);
    }
    std::vector<std::string> info;
    if (enableDrawPosition())
    {
      for (auto& scene : _->scenes)
      {
        auto z = scene->zoomer();
        if (z)
        {
          info.push_back(_->mapMousePositionToScene(z,
                                                    scene->name().c_str(),
                                                    1.0,
                                                    m_debugInfo->curX,
                                                    m_debugInfo->curY));
        }
      }

      _->drawTextAt(canvas, info, m_debugInfo->curX, m_debugInfo->curY);
    }
    _->skiaState.getCanvas()->restore();
  }
}

void VLayer::addRenderItem(std::shared_ptr<Renderable> item)
{
  d_ptr->items.push_back(std::move(item));
}

void VLayer::addScene(std::shared_ptr<Scene> scene)
{
  d_ptr->scenes.push_back(std::move(scene));
}

void VLayer::resize(int w, int h)
{
  VGG_IMPL(VLayer);
  int finalW = w * scale() * dpi();
  int finalH = h * scale() * dpi();
  INFO("resize: [%d, %d]  (%d, %d)", w, h, finalW, finalH);
  _->resizeSkiaSurfaceGL(finalW, finalH);
}
void VLayer::endFrame()
{
  VGG_IMPL(VLayer)
  _->skiaState.getCanvas()->flush();
}

void VLayer::shutdown()
{
}

VLayer::VLayer()
  : d_ptr(new VLayer__pImpl(this))
{
}
VLayer::~VLayer() = default;
} // namespace VGG
  //
