#include "Scene/VGGLayer.h"
#include "Core/Node.h"
#include "Scene/GraphicsContext.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/Renderer.h"
#include "Scene/Zoomer.h"
#include "Scene/Scene.h"
#include "Common/Math.hpp"
#include "CappingProfiler.hpp"

#include "SkiaImpl/VSkiaContext.hpp"
#include "SkiaImpl/VSkiaGL.hpp"

#ifdef VGG_USE_VULKAN
#include "SkiaImpl/VSkiaVK.hpp"
#endif

#include <optional>
#include <sstream>

namespace VGG::layer
{

using namespace VGG::layer::skia_impl;

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
    auto* ctx = reinterpret_cast<ContextInfoGL*>(context()->contextInfo());
    _->skiaContext =
      std::make_unique<SkiaContext>(gl::glContextCreateProc(ctx), gl::glSurfaceCreateProc(), cfg);
  }
  else if (api == EGraphicsAPIBackend::API_VULKAN)
  {
#ifdef VGG_USE_VULKAN
    auto* ctx = reinterpret_cast<ContextInfoVulkan*>(context()->contextInfo());
    _->skiaContext =
      std::make_unique<SkiaContext>(vk::vkContextCreateProc(ctx), vk::vkSurfaceCreateProc(), cfg);
#else
    ASSERT(false && "Vulkan is not support on the platform");
    _->skiaContext = nullptr;
#endif
  }
  else
  {
    ASSERT(false && "Invalid Graphics API backend");
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
    const auto scale = context()->property().dpiScaling * scaleFactor();
    canvas->scale(scale, scale);
    for (auto& scene : _->scenes)
    {
      scene->render(canvas);
    }
    for (auto& item : _->items)
    {
      item->render(canvas);
    }
    if (enableDrawPosition())
    {
      std::vector<std::string> info;
      for (auto& scene : _->scenes)
      {
        auto z = scene->zoomer();
        if (z)
        {
          info.push_back(
            _->mapCanvasPositionToScene(z, scene->name().c_str(), m_position[0], m_position[1]));
        }
      }

      _->drawTextAt(canvas, info, m_position[0], m_position[1]);
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
