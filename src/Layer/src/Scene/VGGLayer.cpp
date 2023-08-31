#include "Scene/VGGLayer.h"
#include "Core/Node.h"
#include "Scene/ContextInfoVulkan.hpp"
#include "Scene/ContextInfoGL.hpp"
#include "Scene/GraphicsContext.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/Renderer.h"
#include "Scene/Zoomer.h"
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
#include "Entry/Common/GPU/Vulkan/VulkanObject.hpp"

namespace VGG::layer
{

template<class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

class SkiaContext
{
public:
  enum class ESkiaGraphicsAPIBackend
  {
    API_OPENGL,
    API_VULKAN
  };

  SkiaContext(const SkiaContext&) = delete;
  SkiaContext& operator=(const SkiaContext&) = delete;
  SkiaContext(SkiaContext&& other) noexcept
    : m_stream(std::move(other.m_stream))
    , m_document(std::move(other.m_document))
    , m_grContext(std::move(other.m_grContext))
    , m_surface(std::move(other.m_surface))
    , m_ctx(std::move(other.m_ctx))
    , m_ctxConfig(std::move(other.m_ctxConfig))
  {
  }
  SkiaContext& operator=(SkiaContext&& other) noexcept
  {
    release();
    *this = SkiaContext(std::move(other));
    return *this;
  }

  void release()
  {
    DEBUG("SkiaContext Releasing ... ");
    // ensure release order
    m_stream = nullptr;
    m_document = nullptr;
    m_surface = nullptr;
    m_grContext = nullptr;
  }

private:
  std::unique_ptr<SkFILEWStream> m_stream;
  sk_sp<SkDocument> m_document;
  sk_sp<GrDirectContext> m_grContext{ nullptr };
  sk_sp<SkSurface> m_surface{ nullptr };
  std::variant<ContextInfoVulkan, ContextInfoGL> m_ctx;
  ContextConfig m_ctxConfig;
  void initContextVK(const ContextInfoVulkan& ctx)
  {
    GrVkBackendContext vkContext;
    vkContext.fInstance = ctx.instance;
    vkContext.fPhysicalDevice = ctx.physicalDevice;
    vkContext.fDevice = ctx.device;
    vkContext.fQueue = ctx.queue;
    vkContext.fGetProc = std::function(
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
    m_grContext = GrDirectContext::MakeVulkan(vkContext);
  }

  void initContextGL(const ContextInfoGL& ctx)
  {
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
    ASSERT(interface);
    if (!interface)
    {
      // return AppError(AppError::Kind::RenderEngineError, "Failed to make skia opengl interface");
    }
    sk_sp<GrDirectContext> grContext = GrDirectContext::MakeGL(interface);
    ASSERT(grContext);
    if (!grContext)
    {
      // return AppError(AppError::Kind::RenderEngineError, "Failed to make skia opengl context.");
    }
    m_grContext = grContext;
  }

  sk_sp<SkSurface> createSurfaceVK(const ContextInfoVulkan& ctx,
                                   const ContextConfig& cfg,
                                   int w,
                                   int h)
  {
    ASSERT(m_grContext);
    GrVkImageInfo vkImageInfo;
    vkImageInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    GrBackendRenderTarget target(w, h, vkImageInfo);
    SkImageInfo info = SkImageInfo::Make(w,
                                         h,
                                         SkColorType::kRGBA_8888_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    auto a = SkSurfaces::RenderTarget(m_grContext.get(), skgpu::Budgeted::kYes, info);
    return a;
  };

  sk_sp<SkSurface> createSurfaceGL(const ContextInfoGL& ctx, const ContextConfig& cfg, int w, int h)
  {
    ASSERT(m_grContext);
    GrGLFramebufferInfo info;
    info.fFBOID = 0;
    info.fFormat = GR_GL_RGBA8;
    GrBackendRenderTarget target(w, h, cfg.multiSample, cfg.stencilBit, info);
    SkSurfaceProps props;
    return SkSurfaces::WrapBackendRenderTarget(m_grContext.get(),
                                               target,
                                               kBottomLeft_GrSurfaceOrigin,
                                               SkColorType::kRGBA_8888_SkColorType,
                                               nullptr,
                                               &props);
  };

public:
  SkiaContext(std::variant<ContextInfoVulkan, ContextInfoGL> context, const ContextConfig& cfg)
    : m_ctx(std::move(context))
    , m_ctxConfig(std::move(cfg))
  {
    std::visit(
      Overloaded{
        [this](const ContextInfoVulkan& ctx)
        {
          initContextVK(ctx);
          m_surface =
            createSurfaceVK(ctx, m_ctxConfig, m_ctxConfig.windowSize[0], m_ctxConfig.windowSize[1]);
        },
        [this](const ContextInfoGL& ctx)
        {
          initContextGL(ctx);
          m_surface =
            createSurfaceGL(ctx, m_ctxConfig, m_ctxConfig.windowSize[0], m_ctxConfig.windowSize[1]);
        } },
      m_ctx);
  }

  void resizeSurface(int w, int h)
  {
    std::visit(Overloaded{ [this, w, h](const ContextInfoVulkan& ctx)
                           { m_surface = createSurfaceVK(ctx, m_ctxConfig, w, h); },
                           [this, w, h](const ContextInfoGL& ctx)
                           { m_surface = createSurfaceGL(ctx, m_ctxConfig, w, h); } },
               m_ctx);
  }

  bool flushAndSubmit()
  {
    auto ok = m_grContext->submit();
    m_grContext->flush();
    return ok;
  }

  SkSurface* surface()
  {
    ASSERT(m_surface);
    return m_surface.get();
  }

  SkCanvas* canvas()
  {
    ASSERT(m_surface);
    return m_surface->getCanvas();
  }

  GrDirectContext* context()
  {
    return m_grContext.get();
  }

  ~SkiaContext()
  {
    release();
  }
};

class VLayer__pImpl
{
  VGG_DECL_API(VLayer);

public:
  Zoomer zoomer;
  // SkiaState skiaState;
  std::unique_ptr<SkiaContext> skiaContext;
  int surfaceWidth;
  int surfaceHeight;
  SkCanvas* canvas{ nullptr };
  std::vector<std::shared_ptr<Renderable>> items;
  std::vector<std::shared_ptr<Scene>> scenes;

  VLayer__pImpl(VLayer* api)
    : q_ptr(api)
  {
  }

  void cleanup()
  {
    skiaContext = nullptr;
  }

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

  const auto& cfg = context()->config();
  const auto api = context()->property().api;
  if (api == EGraphicsAPIBackend::API_OPENGL)
  {
    const auto* ctx = reinterpret_cast<ContextInfoGL*>(context()->contextInfo());
    _->skiaContext = std::make_unique<SkiaContext>(*ctx, cfg);
  }
  else if (api == EGraphicsAPIBackend::API_VULKAN)
  {
    const auto* ctx = reinterpret_cast<ContextInfoVulkan*>(context()->contextInfo());
    _->skiaContext = std::make_unique<SkiaContext>(*ctx, cfg);
  }
  else
  {
    ASSERT(false);
  }
  ASSERT(_->skiaContext);
  return std::nullopt;
}
void VLayer::beginFrame()
{
}

void VLayer::render()
{
  VGG_IMPL(VLayer)
  auto canvas = _->skiaContext->canvas();
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
    canvas->flush();
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
  const int finalW = w;
  const int finalH = h;
  INFO("resize: [%d, %d], actually (%d, %d)", w, h, finalW, finalH);
  _->skiaContext->resizeSurface(finalW, finalH);
}
void VLayer::endFrame()
{
  VGG_IMPL(VLayer)
  ASSERT(context());
  _->skiaContext->flushAndSubmit();
  context()->swap();
}

std::optional<std::vector<char>> VLayer::makeImageSnapshot(const ImageOptions& opts)
{
  VGG_IMPL(VLayer);
  auto surface = _->skiaContext->surface();
  auto ctx = _->skiaContext->context();
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
  VGG_IMPL(VLayer);
  _->cleanup();
}

VLayer::VLayer()
  : d_ptr(new VLayer__pImpl(this))
{
}
VLayer::~VLayer()
{
  DEBUG("VGG Layer releasing...");
}
} // namespace VGG::layer
  //
