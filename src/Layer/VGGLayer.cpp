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
#include <core/SkColor.h>
#include <iterator>
#include <ostream>
#include <sstream>

#include <core/SkPictureRecorder.h>
#include <gpu/GrDirectContext.h>
#include <encode/SkJpegEncoder.h>
#include <encode/SkWebpEncoder.h>

#include "VSkiaGL.hpp" // this header and the skia headers must be included first for ios build

#ifdef VGG_USE_VULKAN
#include "VSkiaVK.hpp"
#endif
#include "Renderer.hpp"
#include "VSkiaContext.hpp"
#include "Stream.hpp"

#include "Layer/VGGLayer.hpp"
#include "Layer/Zoomer.hpp"
#include "Layer/Scene.hpp"
#include "Layer/Graphics/GraphicsLayer.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"
#include "Layer/Graphics/ContextSkBase.hpp"
#include "Layer/Core/TreeNode.hpp"
#include "Utility/CappingProfiler.hpp"

#include <optional>

namespace VGG::layer
{

using namespace VGG::layer::skia_impl;

std::optional<std::vector<char>> encodeImage(
  GrDirectContext* ctx,
  EImageEncode     encode,
  SkImage*         image,
  int              quality)
{
  quality = std::max(std::min(quality, 100), 0);
  if (encode == EImageEncode::IE_PNG)
  {
    SkPngEncoder::Options opt;
    opt.fZLibLevel = std::max(std::min(9, (100 - quality) / 10), 0);
    if (auto data = SkPngEncoder::Encode(ctx, image, opt))
    {
      return std::vector<char>{ data->bytes(), data->bytes() + data->size() };
    }
  }
  else if (encode == EImageEncode::IE_JPEG)
  {
    SkJpegEncoder::Options opt;
    opt.fQuality = quality;
    if (auto data = SkJpegEncoder::Encode(ctx, image, opt))
    {
      return std::vector<char>{ data->bytes(), data->bytes() + data->size() };
    }
  }
  else if (encode == EImageEncode::IE_WEBP)
  {
    SkWebpEncoder::Options opt;
    opt.fQuality = quality;
    if (auto data = SkWebpEncoder::Encode(ctx, image, opt))
    {
      return std::vector<char>{ data->bytes(), data->bytes() + data->size() };
    }
  }
  else
  {
    DEBUG("format (%d) is not supported", (int)encode);
    return std::nullopt;
  }
  DEBUG("Failed to encode image data for type(%d)", (int)encode);
  return std::nullopt;
}

class VLayer__pImpl
{
  VGG_DECL_API(VLayer);

public:
  Zoomer                                   zoomer;
  // SkiaState skiaState;
  std::unique_ptr<SkiaContext>             skiaContext;
  int                                      surfaceWidth;
  int                                      surfaceHeight;
  SkCanvas*                                canvas{ nullptr };
  std::vector<std::shared_ptr<Renderable>> items;
  std::vector<std::shared_ptr<Scene>>      scenes;
  bool                                     invalid{ true };

  VLayer__pImpl(VLayer* api)
    : q_ptr(api)
  {
  }

  void cleanup()
  {
    skiaContext = nullptr;
  }

  void renderInternal(SkCanvas* canvas, bool drawTextInfo)
  {
    ASSERT(canvas);
    canvas->save();
    canvas->clear(SK_ColorWHITE);
    const auto scale = q_ptr->context()->property().dpiScaling * q_ptr->scaleFactor();
    canvas->scale(scale, scale);
    for (auto& scene : scenes)
    {
      scene->render(canvas);
    }
    for (auto& item : items)
    {
      item->render(canvas);
    }
    if (drawTextInfo)
    {
      std::vector<std::string> info;
      for (auto& scene : scenes)
      {
        auto z = scene->zoomer();
        if (z)
        {
          info.push_back(mapCanvasPositionToScene(
            z,
            scene->name().c_str(),
            q_ptr->m_position[0],
            q_ptr->m_position[1]));
        }
      }
      drawTextAt(canvas, info, q_ptr->m_position[0], q_ptr->m_position[1]);
    }
    canvas->restore();
    canvas->flush();
  }

  std::string mapCanvasPositionToScene(
    const Zoomer* zoomer,
    const char*   name,
    int           curMouseX,
    int           curMouseY)
  {
    std::stringstream ss;
    float             windowPos[2] = { (float)curMouseX, (float)curMouseY };
    float             logixXY[2];
    zoomer->mapCanvasPosToLogicalPosition(windowPos, logixXY);
    ss << name << ": (" << (int)logixXY[0] << ", " << (int)logixXY[1] << ")";
    std::string res{ std::istreambuf_iterator<char>{ ss }, {} };
    return res;
  }

  void drawTextAt(
    SkCanvas*                       canvas,
    const std::vector<std::string>& strings,
    int                             curMouseX,
    int                             curMouseY)
  {
    SkPaint textPaint;
    textPaint.setColor(SK_ColorBLACK);
    SkFont        font;
    constexpr int FONTSIZE = 20;
    font.setSize(FONTSIZE);
    int dy = 0;
    for (const auto& text : strings)
    {
      canvas->drawSimpleText(
        text.c_str(),
        text.size(),
        SkTextEncoding::kUTF8,
        curMouseX,
        curMouseY + dy,
        font,
        textPaint);
      dy += FONTSIZE;
    }
  }

  sk_sp<SkImage> makeImage(SkSurface* surface, const ImageOptions& opts)
  {
    return surface->makeImageSnapshot(
      SkIRect::MakeXYWH(opts.position[0], opts.position[1], opts.extend[0], opts.extend[1]));
  }

  void revalidate()
  {
    if (invalid)
    {
      float h = skiaContext->surface()->height();
      float w = skiaContext->surface()->width();
      for (auto& s : scenes)
      {
        s->onViewportChange(Bound{ 0, 0, w, h });
      }
      invalid = false;
    }
  }
  void invalidate()
  {
    invalid = true;
  }
};

std::optional<ELayerError> VLayer::onInit()
{
  VGG_IMPL(VLayer)
  context()->makeCurrent();

  const auto& cfg = context()->config();
  const auto  api = context()->property().api;

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
  else if (api == EGraphicsAPIBackend::API_CUSTOM)
  {
    auto* ctx = reinterpret_cast<SkiaGraphicsContext*>(context());
    _->skiaContext =
      std::make_unique<SkiaContext>(ctx->contextCreateProc(), ctx->surfaceCreateProc(), cfg);
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
  VGG_IMPL(VLayer);
  _->revalidate();
  if (!_->skiaContext->prepareFrame())
    DEBUG("begin frame failed");
}

void VLayer::render()
{
  VGG_IMPL(VLayer)
  SkCanvas* canvas = nullptr;
  canvas = _->skiaContext->canvas();
  Timer t;
  t.start();
  _->renderInternal(canvas, enableDrawPosition());
  t.stop();
  INFO("raster time: %d", (int)t.elapsed().ms());
}

void VLayer::addRenderItem(std::shared_ptr<Renderable> item)
{
  d_ptr->items.push_back(std::move(item));
}

void VLayer::addScene(std::shared_ptr<Scene> scene)
{
  d_ptr->scenes.push_back(std::move(scene));
  d_ptr->invalidate();
}

void VLayer::setScene(std::shared_ptr<Scene> scene)
{
  d_ptr->scenes.clear();
  d_ptr->scenes.push_back(std::move(scene));
  d_ptr->invalidate();
}

void VLayer::resize(int w, int h)
{
  VGG_IMPL(VLayer);
  const int finalW = w;
  const int finalH = h;
  if (finalH > 0 || finalW > 0)
  {
    INFO("resize: [%d, %d], actually (%d, %d)", w, h, finalW, finalH);
    _->skiaContext->resizeSurface(finalW, finalH);
    d_ptr->invalidate();
  }
}
void VLayer::endFrame()
{
  VGG_IMPL(VLayer)
  ASSERT(context());
  _->skiaContext->flushAndSubmit();
  context()->swap();
  _->skiaContext->markSwap();
}

sk_sp<SkPicture> VLayer::makeSkPicture(int width, int height)
{
  VGG_IMPL(VLayer);
  SkPictureRecorder rec;
  auto              canvas = rec.beginRecording(width, height);
  _->renderInternal(canvas, false);
  return rec.finishRecordingAsPicture();
}

std::optional<std::vector<char>> VLayer::makeSKP()
{
  std::stringstream data;
  makeSKP(data);
  return std::vector<char>{ std::istream_iterator<char>(data), std::istream_iterator<char>() };
}

void VLayer::makeSKP(std::ostream& os)
{
  VGG_IMPL(VLayer);
  SkPictureRecorder rec;
  auto              w = _->skiaContext->surface()->width();
  auto              h = _->skiaContext->surface()->height();
  auto              canvas = rec.beginRecording(w, h);
  _->renderInternal(canvas, false);
  auto pic = rec.finishRecordingAsPicture();
  if (pic)
  {
    SkStdOStream skos(os);
    pic->serialize(&skos, 0);
  }
}

void VLayer::makeImageSnapshot(const ImageOptions& opts, std::ostream& os)
{
  if (auto data = makeImageSnapshot(opts))
  {
    os.write(data->data(), data->size());
    return;
  }
  DEBUG("make image snapshot failed");
}

std::optional<std::vector<char>> VLayer::makeImageSnapshot(const ImageOptions& opts)
{
  VGG_IMPL(VLayer);
  auto surface = _->skiaContext->surface();
  _->renderInternal(surface->getCanvas(), false);
  auto ctx = _->skiaContext->context();
  if (
    auto image = surface->makeImageSnapshot(
      SkIRect::MakeXYWH(opts.position[0], opts.position[1], opts.extend[0], opts.extend[1])))
  {
    if (opts.encode != EImageEncode::IE_RAW)
    {
      return encodeImage(ctx, opts.encode, image.get(), opts.quality);
    }
    else
    {
      DEBUG("raw data is not supported now");
      return std::nullopt;
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
