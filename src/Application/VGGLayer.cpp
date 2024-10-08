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
#include <algorithm>
#include <core/SkColor.h>
#include <iterator>
#include <ostream>
#include <sstream>

#include <core/SkPictureRecorder.h>
#include <gpu/GrDirectContext.h>
#include <encode/SkJpegEncoder.h>
#include <encode/SkWebpEncoder.h>
#include "Application/VGGLayer.hpp"

#include "Layer/Core/EventManager.hpp"
#include "Layer/Core/Timer.hpp"
#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/VNode.hpp"
#include "Layer/Core/RasterNode.hpp"
#include "Layer/Raster.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/ViewportNode.hpp"
#include "Layer/Graphics/VSkiaGL.hpp" // this header and the skia headers must be included first for ios build
#include "Layer/SimpleRasterExecutor.hpp"

#include "Layer/Renderer.hpp"
#include "Layer/Stream.hpp"
#include "Layer/Graphics/VSkiaContext.hpp"

#include "Layer/Graphics/GraphicsLayer.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"
#include "Layer/Graphics/ContextSkBase.hpp"
#include "Utility/CappingProfiler.hpp"

#include <optional>

namespace
{

using namespace VGG::layer;

#ifdef VGG_LAYER_DEBUG
inline glm::mat3 getLocalMatrix(PaintNode* node)
{
  ASSERT(node);
  glm::mat3 local = glm::mat3{ 1 };
  while (node)
  {
    local = node->getTransform().matrix() * local;
    node = static_cast<PaintNode*>(node->parent().get());
  }
  return local;
}

inline void drawBounds(PaintNode* node, const glm::mat3& deviceMatrix, SkCanvas* canvas)
{
  ASSERT(canvas);
  if (node)
  {
    auto    localMatrix = getLocalMatrix(node);
    auto    bounds = node->bounds();
    auto    current = toSkMatrix(deviceMatrix * localMatrix);
    auto    deviceBounds = current.mapRect(toSkRect(bounds));
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAlphaf(0.3);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawRect(deviceBounds, paint);
  }
}
#endif

inline void drawTextAt(
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

} // namespace

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
  std::unique_ptr<SkiaContext>          skiaContext;
  std::unique_ptr<SimpleRasterExecutor> simpleRasterExecutor;

  std::vector<std::shared_ptr<Renderable>> items;

  Ref<Viewport>   viewport;
  Ref<RasterNode> rasterNode;

  bool invalid{ true };

  std::unique_ptr<SkPictureRecorder> rec;

#ifdef VGG_LAYER_DEBUG
  sk_sp<SkPicture> clickLayer;
#endif

  SkColor backgroundColor{ SK_ColorWHITE };

  struct DebugConfig
  {
    bool enableDrawClickBounds{ false };
    bool debugMode{ false };
    bool drawPosInfo{ false };
  } debugConfig;

  VLayer__pImpl(VLayer* api)
    : q_ptr(api)
  {
  }

  SkCanvas* layerCanvas()
  {
    auto w = skiaContext->surface()->width();
    auto h = skiaContext->surface()->height();
    if (w == 0 || h == 0)
      return nullptr;
    rec = std::make_unique<SkPictureRecorder>();
    return rec->beginRecording(SkRect::MakeWH(w, h));
  }

  void cleanup()
  {
    simpleRasterExecutor = nullptr;
    skiaContext = nullptr;
  }

  void renderInternal(SkCanvas* canvas, bool drawTextInfo)
  {
    ASSERT(canvas);
    canvas->clear(backgroundColor);
    if (rasterNode)
    {
      Renderer r;
      r = r.createNew(canvas);
      EventManager::pollEvents();
      Revalidation        rev;
      std::vector<Bounds> damageBounds;
      rasterNode->revalidate(&rev, glm::mat3{ 1 });
      rasterNode->raster(mergeBounds(rev.boundsArray()));
      rasterNode->render(&r);
    }
    if (drawTextInfo)
    {
      std::vector<std::string> info;
      drawTextAt(canvas, info, q_ptr->m_position[0], q_ptr->m_position[1]);
    }

    canvas->save();
    const auto scale = q_ptr->context()->property().dpiScaling * q_ptr->scaleFactor();
    canvas->scale(scale, scale);
    for (auto& item : items)
      item->render(canvas);
    canvas->restore();

#ifdef VGG_LAYER_DEBUG
    if (rec)
    {
      clickLayer = rec->finishRecordingAsPicture();
      rec = nullptr;
    }
    if (clickLayer)
    {
      static size_t s_delay = 0;
      canvas->drawPicture(clickLayer);
      s_delay++;
      if (s_delay % 4 == 0)
        clickLayer = nullptr;
    }
    if (q_ptr->debugModeEnabled() && rasterNode)
    {
      Renderer r;
      r = r.createNew(canvas);
      rasterNode->debug(&r);
    }
#endif
    canvas->flush();
  }

  sk_sp<SkImage> makeImage(SkSurface* surface, const ImageOptions& opts)
  {
    return surface->makeImageSnapshot(
      SkIRect::MakeXYWH(opts.position[0], opts.position[1], opts.extend[0], opts.extend[1]));
  }

  void setBackgroundColor(SkColor color)
  {
    backgroundColor = color;
  }
};

void VLayer::setBackgroundColor(uint32_t color)
{
  d_ptr->setBackgroundColor(color);
}

bool VLayer::enableDrawClickBounds() const
{
  return d_ptr->debugConfig.enableDrawClickBounds;
}

void VLayer::setDrawClickBounds(bool enable)
{
  d_ptr->debugConfig.enableDrawClickBounds = enable;
}

void VLayer::setDrawPositionEnabled(bool enable)
{
  d_ptr->debugConfig.drawPosInfo = enable;
}

bool VLayer::enableDrawPosition() const
{
  return d_ptr->debugConfig.drawPosInfo;
}

void VLayer::setDebugModeEnabled(bool enable)
{
  d_ptr->debugConfig.debugMode = enable;
}

bool VLayer::debugModeEnabled()
{
  return d_ptr->debugConfig.debugMode;
}

void VLayer::setScaleFactor(float scale)
{
  d_ptr->viewport->setScale(scale);
}

float VLayer::scaleFactor() const
{
  return d_ptr->viewport->getScale();
}

Viewport* VLayer::viewport()
{
  return d_ptr->viewport.get();
}

std::optional<ELayerError> VLayer::onInit()
{
  VGG_IMPL(VLayer)
  context()->makeCurrent();

  const auto& cfg = context()->config();
  const auto  api = context()->property().api;

  d_ptr->viewport = Viewport::Make(context()->property().dpiScaling);

  if (api == EGraphicsAPIBackend::API_OPENGL)
  {
    auto* ctx = reinterpret_cast<ContextInfoGL*>(context()->contextInfo());
    _->skiaContext =
      std::make_unique<SkiaContext>(gl::glContextCreateProc(ctx), gl::glSurfaceCreateProc(), cfg);
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
  _->simpleRasterExecutor = std::make_unique<SimpleRasterExecutor>(_->skiaContext->context());
  return std::nullopt;
}
void VLayer::beginFrame()
{
  VGG_IMPL(VLayer);
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
  auto tt = (int)t.duration().ms();
  INFO("render time: %d ms", tt);
}

void VLayer::addRenderItem(std::shared_ptr<Renderable> item)
{
  d_ptr->items.push_back(std::move(item));
}

void VLayer::setRenderNode(Ref<RenderNode> node)
{
  d_ptr->rasterNode = raster::makeEmptyRaster(std::move(node));
}

void VLayer::setRenderNode(Ref<ZoomerNode> transform, Ref<RenderNode> node)
{
  d_ptr->rasterNode = raster::make(
    d_ptr->simpleRasterExecutor.get(),
    d_ptr->viewport,
    std::move(transform),
    std::move(node));
}

} // namespace VGG::layer

namespace
{
PaintNode* g_nodeAtResult = nullptr;
}; // namespace

PaintNode* VLayer::nodeAt(int x, int y)
{
  if (d_ptr->rasterNode)
  {
    auto p = glm::vec3{ x, y, 1 };
    g_nodeAtResult = nullptr;
    d_ptr->rasterNode->nodeAt(
      p.x,
      p.y,
      [](RenderNode* node, const RenderNode::NodeAtContext* ctx)
      {
        auto frameNode = static_cast<FrameNode*>(node);
        auto paintNode = frameNode->node();
        paintNode->nodeAt(
          ctx->localX,
          ctx->localY,
          [](PaintNode* p, const PaintNode::NodeAtContext* ctx)
          {
            if (!g_nodeAtResult)
              g_nodeAtResult = p;
          },
          nullptr);
      },
      nullptr);
    VGG_LAYER_DEBUG_CODE(if (g_nodeAtResult && d_ptr->debugConfig.enableDrawClickBounds) {
      drawBounds(g_nodeAtResult, glm::mat3{ 1 }, d_ptr->layerCanvas());
    });
    return g_nodeAtResult;
  }
  return nullptr;
}

void VLayer::nodeAt(int x, int y, PaintNode::NodeVisitor visitor)
{
  if (d_ptr->rasterNode)
  {
    auto p = glm::vec3{ x, y, 1 };
    d_ptr->rasterNode->nodeAt(
      p.x,
      p.y,
      [](RenderNode* node, const RenderNode::NodeAtContext* ctx)
      {
        auto paintNode = static_cast<FrameNode*>(node)->node();
        paintNode->nodeAt(ctx->localX, ctx->localY, (PaintNode::NodeVisitor)ctx->userData, nullptr);
      },
      (void*)visitor);
  }
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
    _->viewport->setViewport(Bounds{ 0, 0, (float)finalW, (float)finalH });
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
// namespace VGG::layer
//
