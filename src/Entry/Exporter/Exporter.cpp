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
#include "ContextVk.hpp"
#include "Domain/Layout/Rule.hpp"
#include "Layer/Core/DefaultResourceProvider.hpp"
#include "Layer/Core/ResourceManager.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/LayerCache.h"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Layer/DocBuilder.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Utility/ConfigManager.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/Model/StructModel.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Scene.hpp"
#include "Layer/Exporter/SVGExporter.hpp"
#include "Layer/Exporter/PDFExporter.hpp"

#include "VGG/Exporter/ImageExporter.hpp"
#include "VGG/Exporter/SVGExporter.hpp"
#include "VGG/Exporter/PDFExporter.hpp"
#include "VGG/Exporter/Type.hpp"

#include <VGGVersion_generated.h>

#include <cmath>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <variant>
#include <exception>
#include <memory>
#include <optional>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>

static constexpr int MAX_WIDTH = 8192;
static constexpr int MAX_HEIGHT = 8192;
namespace VGG::exporter
{

void getMaxSurfaceSize(int resolutionLevel, float* maxSurfaceSize)
{
  switch (resolutionLevel)
  {
    case 0:
    {
      maxSurfaceSize[0] = 1024;
      maxSurfaceSize[1] = 1024;
    }
    break;
    case 1:
    {
      maxSurfaceSize[0] = 1024;
      maxSurfaceSize[1] = 1024;
    }
    break;
    case 2:
    {
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
    break;
    case 3:
    {
      maxSurfaceSize[0] = 4096;
      maxSurfaceSize[1] = 4096;
    }
    break;
    case 4:
    {
      maxSurfaceSize[0] = 8192;
      maxSurfaceSize[1] = 8192;
    }
    break;
    default:
    {
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
  }
}

float calcScaleFactor(
  float  inputWidth,
  float  inputHeight,
  float  maxWidth,
  float  maxHeight,
  float& outWidth,
  float& outHeight)
{
  auto widthScale = maxWidth / inputWidth;
  auto heightScale = maxHeight / inputHeight;
  // float outputSize[2] = { 0.f, 0.f };
  if (widthScale < heightScale)
  {
    outWidth = maxWidth;
    outHeight = widthScale * inputHeight;
  }
  else
  {
    outWidth = heightScale * inputWidth;
    outHeight = maxHeight;
  }
  return widthScale > heightScale ? heightScale : widthScale;
}

layer::EImageEncode toEImageEncode(EImageType type)
{
  switch (type)
  {
    case PNG:
      return layer::EImageEncode::IE_PNG;
    case JPEG:
      return layer::EImageEncode::IE_JPEG;
    case WEBP:
      return layer::EImageEncode::IE_WEBP;
    default:
      return layer::EImageEncode::IE_PNG;
  }
}

class Exporter__pImpl
{
  Exporter* q_api; // NOLINT
public:
  std::shared_ptr<VkGraphicsContext> ctx;
  std::shared_ptr<layer::VLayer>     layer;
  std::shared_ptr<Scene>             scene;
  OutputCallback                     outputCallback;
  Exporter__pImpl(Exporter* api)
    : q_api(api)
  {
  }

  void resize(int w, int h)
  {
    layer->resize(w, h);
  }

  [[deprecated]] std::optional<std::vector<char>> render(
    std::shared_ptr<Scene>     scene,
    float                      scale,
    const layer::ImageOptions& opts,
    IteratorResult::TimeCost&  cost)
  {
    // auto id = scene->frame(scene->currentPage())->guid();
    layer->setScene(std::move(scene));
    layer->setScaleFactor(scale);
    // begin render one frame
    {
      layer::ScopedTimer t([&](auto d) { cost.render = d.s(); });
      layer->beginFrame();
      layer->render();
      layer->endFrame();
    }
    // end render one frame
    std::optional<std::vector<char>> img;
    {
      layer::ScopedTimer t([&](auto d) { cost.encode = d.s(); });
      img = layer->makeImageSnapshot(opts);
    }
    return img;
  }

  std::optional<std::vector<char>> render(
    layer::Ref<layer::Frame>   f,
    float                      scale,
    const layer::ImageOptions& opts,
    IteratorResult::TimeCost&  cost)
  {
    // layer->setScene(std::move(scene));
    layer->setRenderNode(f);
    layer->setScaleFactor(scale);
    // begin render one frame
    {
      layer::ScopedTimer t([&](auto d) { cost.render = d.s(); });
      layer->beginFrame();
      layer->render();
      layer->endFrame();
    }
    // end render one frame
    std::optional<std::vector<char>> img;
    {
      layer::ScopedTimer t([&](auto d) { cost.encode = d.s(); });
      img = layer->makeImageSnapshot(opts);
    }
    return img;
  }
};

void setGlobalConfig(const std::string& fileName)
{
  Config::readGlobalConfig(fileName);
}

Exporter::Exporter()
  : d_impl(new Exporter__pImpl(this))
{
  layer::ContextConfig cfg;
  d_impl->ctx = std::make_shared<VkGraphicsContext>();
  d_impl->layer = std::make_shared<layer::VLayer>();
  cfg.stencilBit = 8;
  cfg.multiSample = 4;
  d_impl->ctx->init(cfg);
  d_impl->layer->init(d_impl->ctx.get());
}

void Exporter::info(ExporterInfo* info)
{
  if (info)
  {
    info->graphicsInfo = d_impl->ctx->vulkanInfo();
#ifdef GIT_COMMIT
    info->buildCommit = GIT_COMMIT;
#else
    info->buildCommit = "";
#endif
  }
}

Exporter::~Exporter() = default;

void Exporter::setOutputCallback(OutputCallback callback)
{
  d_impl->outputCallback = std::move(callback);
}

class IteratorImplBase
{
public:
  float                                  maxSurfaceSize[2];
  int                                    index{ 0 };
  std::vector<layer::FramePtr>           frames; // FIXME:: use const
  std::vector<layer::FramePtr>::iterator iter;

  void initInternal(
    nlohmann::json      json,
    nlohmann::json      layout,
    const ExportOption& exportOpt,
    BuilderResult&      result)
  {
    // scene = std::make_shared<Scene>();
    auto res = VGG::entry::DocBuilder::builder()
                 .setDocument(std::move(json))
                 .setLayout(std::move(layout))
                 .setExpandEnabled(exportOpt.enableExpand)
                 .setLayoutEnabled(exportOpt.enableLayout)
                 .build();
    BuilderResult::TimeCost cost;
    cost.layout = res.timeCost.layout.s();
    cost.expand = res.timeCost.expand.s();

    std::vector<layer::StructFrameObject> frames;
    for (auto& f : *res.doc)
    {
      if (f->type() == VGG::Domain::Element::EType::FRAME)
      {
        frames.emplace_back(layer::StructFrameObject(f.get()));
      }
    }
    auto sceneBuilderResult = VGG::layer::SceneBuilder::builder()
                                .setResetOriginEnable(true)
                                .setCheckVersion(VGG_PARSE_FORMAT_VER_STR)
                                .setAllocator(layer::getGlobalMemoryAllocator())
                                .build<layer::StructModelFrame>(std::move(frames));
    if (sceneBuilderResult.type)
    {
      if (*sceneBuilderResult.type == VGG::layer::SceneBuilderResult::EResultType::VERSION_MISMATCH)
      {
        result.type = BuilderResult::VERSION_MISMATCH;
      }
    }
    if (sceneBuilderResult.root)
    {
      namespace sv = std::views;
      auto ranges =
        std::move(*sceneBuilderResult.root) | sv::filter([](auto& f) { return f->isVisible(); });
      this->frames = std::vector<layer::FramePtr>(ranges.begin(), ranges.end());
      iter = this->frames.begin();
    }
    result.timeCost = cost;
    index = 0;
  }

  IteratorImplBase(nlohmann::json json, nlohmann::json layout)
  {
    BuilderResult result;
    initInternal(std::move(json), std::move(layout), ExportOption(), result);
  }
  IteratorImplBase(
    nlohmann::json      json,
    nlohmann::json      layout,
    const ExportOption& exportOpt,
    BuilderResult&      result)
  {
    initInternal(std::move(json), std::move(layout), exportOpt, result);
  }
  ~IteratorImplBase() = default;
};

class ImageIteratorImpl : public IteratorImplBase
{
public:
  Exporter&               exporter;
  ImageOption::SizePolicy size;
  ImageIteratorImpl(
    Exporter&               exporter,
    nlohmann::json          json,
    nlohmann::json          layout,
    ImageOption::SizePolicy size,
    const ExportOption&     opt,
    BuilderResult&          result)
    : IteratorImplBase(std::move(json), std::move(layout), opt, result)
    , exporter(exporter)
    , size(size)
  {
    exporter.d_impl->resize(MAX_WIDTH, MAX_HEIGHT);
  }

  bool next(
    std::string&              key,
    std::vector<char>&        image,
    EImageType                type,
    int                       quality,
    IteratorResult::TimeCost& cost)
  {
    if (iter == frames.end())
    {
      return false;
    }
    auto f = *iter;
    if (!f)
      return false;
    f->revalidate();
    const auto b = f->bounds();
    const auto id = f->guid();
    const auto w = b.size().x;
    const auto h = b.size().y;

    auto          state = exporter.d_impl.get();
    float         actualSize[2];
    float         scale = 1.0;
    constexpr int MAX_SIDE = std::min(MAX_WIDTH, MAX_HEIGHT);
    std::visit(
      layer::Overloaded{ [&](const ImageOption::ScaleDetermine& s)
                         {
                           auto maxScale = MAX_SIDE / std::max(w, h);
                           scale = s.value > maxScale ? maxScale : s.value;
                           actualSize[0] = scale * w;
                           actualSize[1] = scale * h;
                         },
                         [&](const ImageOption::WidthDetermine& width)
                         {
                           auto maxSide = width.value > MAX_SIDE ? MAX_SIDE : width.value;
                           scale = std::min({ MAX_HEIGHT / w, MAX_WIDTH / h, maxSide / w });
                           actualSize[0] = scale * w;
                           actualSize[1] = scale * h;
                         },
                         [&](const ImageOption::HeightDetermine& height)
                         {
                           auto maxSide = height.value > MAX_SIDE ? MAX_SIDE : height.value;
                           scale = std::min({ MAX_HEIGHT / w, MAX_WIDTH / h, maxSide / h });
                           actualSize[0] = scale * w;
                           actualSize[1] = scale * h;
                         },
                         [&, this](const ImageOption::LevelDetermine& level)
                         {
                           getMaxSurfaceSize(level.value, maxSurfaceSize);
                           scale = calcScaleFactor(
                             w,
                             h,
                             maxSurfaceSize[0],
                             maxSurfaceSize[1],
                             actualSize[0],
                             actualSize[1]);
                         } },
      size);
    layer::ImageOptions opts;
    opts.encode = toEImageEncode(type);
    opts.position[0] = 0;
    opts.position[1] = 0;
    opts.extend[0] = std::lroundf(actualSize[0]);
    opts.extend[1] = std::lroundf(actualSize[1]);
    INFO(
      "scale: %f, [%d(%f), %d(%f)]",
      scale,
      opts.extend[0],
      actualSize[0],
      opts.extend[1],
      actualSize[1]);
    opts.quality = quality;
    f->resetToOrigin(true);
    auto res = state->render(f, scale, opts, cost);
    if (!res.has_value())
    {
      return false;
    }
    key = std::move(id);
    image = std::move(res.value());
    ++iter;
    return true;
  }
};

// ImageIterator
bool ImageIterator::next(std::string& key, std::vector<char>& image)
{
  IteratorResult::TimeCost cost;
  return d_impl->next(key, image, m_opts.type, m_opts.imageQuality, cost);
}

IteratorResult ImageIterator::next()
{
  std::string              key;
  std::vector<char>        image;
  IteratorResult::TimeCost cost;
  IteratorResult           res(d_impl->next(key, image, m_opts.type, m_opts.imageQuality, cost));
  res.data = { std::move(key), std::move(image) };
  res.timeCost = cost;
  return res;
}

ImageIterator::ImageIterator(ImageIterator&& other) noexcept
  : d_impl(std::move(other.d_impl))
  , m_opts(other.m_opts)
{
}

ImageIterator::ImageIterator(
  Exporter&           exporter,
  nlohmann::json      design,
  nlohmann::json      layout,
  const ImageOption&  opt,
  const ExportOption& exportOpt,
  BuilderResult&      result)
  : d_impl(std::make_unique<ImageIteratorImpl>(
      exporter,
      std::move(design),
      std::move(layout),
      opt.size,
      exportOpt,
      result))
  , m_opts(opt)
{
}

ImageIterator::~ImageIterator() = default;

SVGIterator::SVGIterator(nlohmann::json design, nlohmann::json layout)
  : d_impl(std::make_unique<IteratorImplBase>(std::move(design), std::move(layout)))
{
}

SVGIterator::SVGIterator(
  nlohmann::json      design,
  nlohmann::json      layout,
  const ExportOption& opt,
  BuilderResult&      result)
  : d_impl(std::make_unique<IteratorImplBase>(std::move(design), std::move(layout), opt, result))
{
}

SVGIterator::SVGIterator(SVGIterator&& other) noexcept
  : d_impl(std::move(other.d_impl))
{
}
bool SVGIterator::next(std::string& key, std::vector<char>& data)
{
  if (d_impl->iter == d_impl->frames.end())
  {
    return false;
  }
  auto f = *d_impl->iter;
  if (!f)
    return false;
  f->revalidate();
  auto b = f->bounds();
  auto id = f->guid();

  layer::exporter::SVGOptions opts;
  opts.extend[0] = b.width();
  opts.extend[1] = b.height();
  auto res = layer::exporter::makeSVG(f, opts);
  if (!res.has_value())
  {
    return false;
  }
  key = std::move(id);
  data = std::move(res.value());
  ++d_impl->iter;
  return true;
}

IteratorResult SVGIterator::next()
{
  std::string       key;
  std::vector<char> image;
  layer::Timer      t;
  t.start();
  IteratorResult res(next(key, image));
  t.stop();
  res.data = { std::move(key), std::move(image) };
  IteratorResult::TimeCost cost;
  cost.render = t.duration().s();
  cost.encode = 0.f;
  res.timeCost = cost;
  return res;
}
SVGIterator::~SVGIterator() = default;

PDFIterator::PDFIterator(nlohmann::json design, nlohmann::json layout)
  : d_impl(std::make_unique<IteratorImplBase>(std::move(design), std::move(layout)))
{
}

PDFIterator::PDFIterator(
  nlohmann::json      design,
  nlohmann::json      layout,
  const ExportOption& opt,
  BuilderResult&      result)
  : d_impl(std::make_unique<IteratorImplBase>(std::move(design), std::move(layout), opt, result))
{
}

PDFIterator::PDFIterator(PDFIterator&& other) noexcept
  : d_impl(std::move(other.d_impl))
{
}
bool PDFIterator::next(std::string& key, std::vector<char>& data)
{
  if (d_impl->iter == d_impl->frames.end())
  {
    return false;
  }
  auto f = *d_impl->iter;
  if (!f)
    return false;
  f->revalidate();
  auto                        id = f->guid();
  auto                        b = f->bounds();
  layer::exporter::PDFOptions opts;
  opts.extend[0] = b.width();
  opts.extend[1] = b.height();
  auto res = layer::exporter::makePDF(f, opts);
  if (!res.has_value())
  {
    return false;
  }
  key = std::move(id);
  data = std::move(res.value());
  ++d_impl->iter;
  return true;
}

IteratorResult PDFIterator::next()
{
  std::string       key;
  std::vector<char> image;
  layer::Timer      t;
  t.start();
  IteratorResult res(next(key, image));
  t.stop();
  res.data = { std::move(key), std::move(image) };
  IteratorResult::TimeCost cost;
  cost.render = t.duration().s();
  cost.encode = 0.f;
  res.timeCost = cost;
  return res;
}

PDFIterator::~PDFIterator() = default;
}; // namespace VGG::exporter
