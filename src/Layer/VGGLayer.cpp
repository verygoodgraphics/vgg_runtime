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
// #include <format>
#include <iterator>
#include <ostream>
#include <sstream>

#include <core/SkPictureRecorder.h>
#include <gpu/GrDirectContext.h>
#include <encode/SkJpegEncoder.h>
#include <encode/SkWebpEncoder.h>

#include "Layer/Core/TransformNode.hpp"
#include "Layer/Core/VNode.hpp"
#include "Layer/RasterNode.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/ViewportNode.hpp"
#include "VSkiaGL.hpp" // this header and the skia headers must be included first for ios build

#ifdef VGG_USE_VULKAN
#include "VSkiaVK.hpp"
#endif
#include "Renderer.hpp"
#include "VSkiaContext.hpp"
#include "Stream.hpp"

#include "Layer/VGGLayer.hpp"
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
    local = node->transform().matrix() * local;
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
  std::unique_ptr<SkiaContext> skiaContext;

  std::vector<std::shared_ptr<Renderable>> items;

  Ref<ViewportNode> viewport;
  Ref<ZoomerNode>   zoomerNode;

  Ref<RenderNode> node;
  float           preScale{ 1.0 };
  bool            invalid{ true };

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
    skiaContext = nullptr;
  }

  glm::mat3 matrix() const
  {
    auto scale = q_ptr->context()->property().dpiScaling * q_ptr->scaleFactor();
    return glm::mat3{ scale, 0, 0, 0, scale, 0, 0, 0, 1 };
  }

  glm::mat3 invMatrix() const
  {
    auto scale = 1.0 / q_ptr->context()->property().dpiScaling * q_ptr->scaleFactor();
    return glm::mat3{ scale, 0, 0, 0, scale, 0, 0, 0, 1 };
  }

  void renderInternal(SkCanvas* canvas, bool drawTextInfo)
  {
    ASSERT(canvas);
    canvas->save();
    canvas->clear(backgroundColor);
    canvas->concat(toSkMatrix(matrix()));
    if (node)
    {
      Renderer r;
      r = r.createNew(canvas);
      node->render(&r);
    }
    if (drawTextInfo)
    {
      std::vector<std::string> info;
      drawTextAt(canvas, info, q_ptr->m_position[0], q_ptr->m_position[1]);
    }
    for (auto& item : items)
    {
      item->render(canvas);
    }

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
    if (q_ptr->debugModeEnabled())
    {
      Renderer r;
      r = r.createNew(canvas);
      node->debug(&r);
    }
#endif
    canvas->restore();
    canvas->flush();
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
      invalid = false;
    }
  }
  void invalidate()
  {
    invalid = true;
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
  if (d_ptr->preScale == scale)
    return;
  d_ptr->preScale = scale;
  d_ptr->viewport->setDPI(context()->property().dpiScaling * scale);
}

float VLayer::scaleFactor() const
{
  return d_ptr->preScale;
}

std::optional<ELayerError> VLayer::onInit()
{
  VGG_IMPL(VLayer)
  context()->makeCurrent();

  const auto& cfg = context()->config();
  const auto  api = context()->property().api;

  d_ptr->viewport = ViewportNode::Make(d_ptr->preScale * context()->property().dpiScaling);

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
  if (_->node)
  {
    _->node->revalidate();
  }
  Timer t;
  t.start();
  _->renderInternal(canvas, enableDrawPosition());
  t.stop();
  auto tt = (int)t.duration().ms();
  if (tt > 20)
  {
    INFO("render time: %d ms", tt);
  }
}

void VLayer::addRenderItem(std::shared_ptr<Renderable> item)
{
  d_ptr->items.push_back(std::move(item));
}

void VLayer::setRenderNode(Ref<ZoomerNode> transform, Ref<RenderNode> node)
{
  d_ptr->zoomerNode = transform;
  auto rasterNode = RasterNode::Make(
    d_ptr->skiaContext->context(),
    d_ptr->viewport,
    std::move(transform),
    std::move(node));
  d_ptr->node = std::move(rasterNode);
}

void VLayer::setRenderNode(Ref<RenderNode> node)
{
  if (d_ptr->node != node)
    d_ptr->node = std::move(node);
}
} // namespace VGG::layer

namespace
{
PaintNode* g_nodeAtResult = nullptr;
}; // namespace

PaintNode* VLayer::nodeAt(int x, int y)
{
  if (d_ptr->node)
  {
    auto p = d_ptr->invMatrix() * glm::vec3{ x, y, 1 };
    g_nodeAtResult = nullptr;
    glm::mat3 deviceMatrix{ 1.f };
    if (d_ptr->viewport)
      deviceMatrix *= d_ptr->viewport->getMatrix();
    if (d_ptr->zoomerNode)
    {
      deviceMatrix *= d_ptr->zoomerNode->getMatrix();
    }

    d_ptr->node->nodeAt(
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
      drawBounds(g_nodeAtResult, deviceMatrix, d_ptr->layerCanvas());
    });
    return g_nodeAtResult;
  }
  return nullptr;
}

void VLayer::nodeAt(int x, int y, PaintNode::NodeVisitor visitor)
{
  if (d_ptr->node)
  {
    auto p = d_ptr->invMatrix() * glm::vec3{ x, y, 1 };
    d_ptr->node->nodeAt(
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
// namespace VGG::layer
//
