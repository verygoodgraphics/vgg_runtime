#include "Scene/VGGLayer.h"
#include "Core/Node.h"
#include "Scene/GraphicsContext.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/Renderer.h"
#include "Scene/Zoomer.h"
#include "GPU/vk/VulkanObject.hpp"
#include <gpu/GpuTypes.h>
#include <gpu/GrTypes.h>
#include <gpu/vk/GrVkTypes.h>
#include <optional>
#include <sstream>
#include <vulkan/vulkan_core.h>

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

#include "gpu/vk/GrVkBackendContext.h"

#include <queue>

#define USE_VULKAN

namespace VGG::layer
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

  GrVkBackendContext vkContext;

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
  Zoomer zoomer;
  SkiaState skiaState;
  int surfaceWidth;
  int surfaceHeight;
  SkCanvas* canvas{ nullptr };
  std::vector<std::shared_ptr<Renderable>> items;
  std::vector<std::shared_ptr<Scene>> scenes;

#ifdef USE_VULKAN
  std::shared_ptr<vk::VkInstanceObject> vkInstance;
  std::shared_ptr<vk::VkPhysicalDeviceObject> vkPhysicalDevice;
  std::shared_ptr<vk::VkDeviceObject> vkDevice;
#endif
  VLayer__pImpl(VLayer* api)
    : q_ptr(api)
  {
  }

  void updateSkiaEngineGL()
  {
#ifdef USE_VULKAN
    updateSkiaEngineVK();
#else
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
#endif
  }

  void resizeSkiaSurfaceGL(int w, int h)
  {

#ifdef USE_VULKAN
    resizeSkiaSurfaceVk(w, h);
#else
    skiaState.surface = createSkiaSurfaceGL(w, h);
    surfaceWidth = w;
    surfaceHeight = h;
#endif
  }

  sk_sp<SkSurface> createSkiaSurfaceGL(int w, int h)
  {

#ifdef USE_VULKAN
    return createSkiaSurfaceVK(w, h);
#else
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
    const auto& cfg = q_ptr->context()->config();
    GrBackendRenderTarget target(w, h, cfg.multiSample, cfg.stencilBit, info);

    SkSurfaceProps props;
    return SkSurfaces::WrapBackendRenderTarget(skiaState.grContext.get(),
                                               target,
                                               kBottomLeft_GrSurfaceOrigin,
                                               SkColorType::kRGBA_8888_SkColorType,
                                               nullptr,
                                               &props);
#endif
  }

#ifdef USE_VULKAN

  void initVulkanObject()
  {
    vkInstance = std::make_shared<vk::VkInstanceObject>();
    skiaState.vkContext.fInstance = VkInstance(*vkInstance);
    vkPhysicalDevice = std::make_shared<vk::VkPhysicalDeviceObject>(vkInstance);
    skiaState.vkContext.fPhysicalDevice = VkPhysicalDevice(*vkPhysicalDevice);
    vkDevice = std::make_shared<vk::VkDeviceObject>(vkPhysicalDevice);
    skiaState.vkContext.fDevice = VkDevice(*vkDevice);
    skiaState.vkContext.fQueue = vkDevice->graphicsQueue;
    skiaState.vkContext.fGetProc = std::function(
      [](const char* name, VkInstance instance, VkDevice dev) -> PFN_vkVoidFunction
      {
        if (dev == VK_NULL_HANDLE && instance == VK_NULL_HANDLE)
        {
          return vkGetInstanceProcAddr(VK_NULL_HANDLE, name);
        }
        if (dev != VK_NULL_HANDLE)
        {
          return vkGetDeviceProcAddr(dev, name);
        }
        else if (instance != VK_NULL_HANDLE)
        {
          return vkGetInstanceProcAddr(instance, name);
        }
        ASSERT(" invalid option");
        return nullptr;
      });

    ASSERT(vkInstance);
    ASSERT(vkPhysicalDevice);
    ASSERT(vkDevice);
  }
  void updateSkiaEngineVK()
  {
    // init vkContext;
    skiaState.grContext = GrDirectContext::MakeVulkan(skiaState.vkContext);
  }

  void resizeSkiaSurfaceVk(int w, int h)
  {
    skiaState.surface = createSkiaSurfaceVK(w, h);
    surfaceWidth = w;
    surfaceHeight = h;
  }
  sk_sp<SkSurface> createSkiaSurfaceVK(int w, int h)
  {
    GrVkImageInfo vkImageInfo;
    vkImageInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    const auto& cfg = q_ptr->context()->config();
    GrBackendRenderTarget target(w, h, vkImageInfo);

    // GrBackendFormat f = GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
    // auto a =
    //   skiaState.grContext->createBackendTexture(w, h, f, GrMipmapped::kNo, GrRenderable::kYes);
    //
    // SkSurfaceProps props;
    // auto r = SkSurfaces::WrapBackendRenderTarget(skiaState.grContext.get(),
    //                                              target,
    //                                              kBottomLeft_GrSurfaceOrigin,
    //                                              SkColorType::kRGBA_8888_SkColorType,
    //                                              nullptr,
    //                                              &props);

    SkImageInfo info = SkImageInfo::Make(w,
                                         h,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    auto a = SkSurfaces::RenderTarget(skiaState.grContext.get(), skgpu::Budgeted::kNo, info);
    return a;
  }

#endif

  std::string mapCanvasPositionToScene(const Zoomer* zoomer,
                                       const char* name,
                                       int curMouseX,
                                       int curMouseY)
  {
    std::stringstream ss;
    float windowPos[2] = { (float)curMouseX, (float)curMouseY };
    float logixXY[2];
    zoomer->mapCanvasPosToLogicalPosition(windowPos, logixXY);
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
};

std::optional<ELayerError> VLayer::onInit()
{
  VGG_IMPL(VLayer)
  context()->makeCurrent();
#ifdef USE_VULKAN
  _->initVulkanObject();
#endif
  _->updateSkiaEngineGL();
  const auto& cfg = context()->config();
  resize(cfg.windowSize[0], cfg.windowSize[1]);
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
    canvas->scale(m_scale, m_scale);
    for (auto& scene : _->scenes)
    {
      scene->render(canvas);
    }
    for (auto& item : _->items)
    {
      item->render(canvas);
    }
    std::vector<std::string> info;
    if (enableDrawPosition())
    {
      const float resolutionScale = context()->property().resolutionScale;
      const float canvasPosX = m_mousePosition[0] * resolutionScale;
      const float canvasPosY = m_mousePosition[1] * resolutionScale;
      for (auto& scene : _->scenes)
      {
        auto z = scene->zoomer();
        if (z)
        {
          info.push_back(
            _->mapCanvasPositionToScene(z, scene->name().c_str(), canvasPosX, canvasPosY));
        }
      }

      _->drawTextAt(canvas, info, canvasPosX, canvasPosY);
    }
    canvas->restore();
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
  const auto f = 1.0;
  const int finalW = w * f;
  const int finalH = h * f;
  INFO("resize: [%d, %d], actually (%d, %d)", w, h, finalW, finalH);
  _->resizeSkiaSurfaceGL(finalW, finalH);
}
void VLayer::endFrame()
{
  VGG_IMPL(VLayer)
  ASSERT(context());
  _->skiaState.getCanvas()->flush();
  context()->swap();
}

std::optional<std::vector<char>> VLayer::makeImageSnapshot(const ImageOptions& opts)
{
  VGG_IMPL(VLayer);
  auto surface = _->skiaState.surface;
  auto ctx = _->skiaState.grContext.get();
  if (auto image = surface->makeImageSnapshot(
        SkIRect::MakeXYWH(opts.position[0], opts.position[1], opts.extend[0], opts.extend[1])))
  {
    SkPngEncoder::Options opt;
    opt.fZLibLevel = std::max(std::min(9, (100 - opts.quality) / 10), 0);
    if (auto data = SkPngEncoder::Encode(ctx, image.get(), opt))
    {
      return std::vector<char>{ data->bytes(), data->bytes() + data->size() };
    }
    else
    {
      DEBUG("Failed to encode image data");
    }
  }
  return std::nullopt;
}

void VLayer::shutdown()
{
}

VLayer::VLayer()
  : d_ptr(new VLayer__pImpl(this))
{
}
VLayer::~VLayer() = default;
} // namespace VGG::layer
  //
