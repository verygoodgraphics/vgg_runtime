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
#include "ContextVk.hpp"
#include "Domain/Layout/Rule.hpp"
#include "Layer/Core/VUtils.hpp"
#include "Layer/Memory/AllocatorImpl.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Layer/DocBuilder.hpp"
#include "Layer/SceneBuilder.hpp"
#include "Utility/ConfigManager.hpp"
#include "Layer/Core/PaintNode.hpp"
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
#include <variant>
#include <exception>
#include <memory>
#include <optional>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>

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
    case 5:
    {
      maxSurfaceSize[0] = 16384;
      maxSurfaceSize[1] = 16384;
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

  std::optional<std::vector<char>> render(
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
      ScopedTimer t([&](auto d) { cost.render = d.s(); });
      layer->beginFrame();
      layer->render();
      layer->endFrame();
    }
    // end render one frame
    std::optional<std::vector<char>> img;
    {
      ScopedTimer t([&](auto d) { cost.encode = d.s(); });
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
  float                  maxSurfaceSize[2];
  int                    totalFrames{ 0 };
  int                    index{ 0 };
  std::shared_ptr<Scene> scene;

  void initInternal(
    nlohmann::json      json,
    nlohmann::json      layout,
    Resource            resource,
    const ExportOption& exportOpt,
    BuilderResult&      result)
  {
    scene = std::make_shared<Scene>();
    auto res = VGG::entry::DocBuilder::builder()
                 .setDocument(std::move(json))
                 .setLayout(std::move(layout))
                 .setExpandEnabled(exportOpt.enableExpand)
                 .setLayoutEnabled(exportOpt.enableLayout)
                 .build();
    BuilderResult::TimeCost cost;
    cost.layout = res.timeCost.layout.s();
    cost.expand = res.timeCost.expand.s();
    auto sceneBuilderResult = VGG::layer::SceneBuilder::builder()
                                .setResetOriginEnable(true)
                                .setCheckVersion(VGG_PARSE_FORMAT_VER_STR)
                                .setDoc(std::move(*res.doc))
                                .setAllocator(VGG_GlobalMemoryAllocator())
                                .build();
    if (sceneBuilderResult.type)
    {
      if (*sceneBuilderResult.type == VGG::layer::SceneBuilderResult::EResultType::VERSION_MISMATCH)
      {
        result.type = BuilderResult::VERSION_MISMATCH;
      }
    }
    if (sceneBuilderResult.root)
    {
      scene->setSceneRoots(std::move(*sceneBuilderResult.root));
    }
    result.timeCost = cost;
    scene->setResRepo(std::move(resource));
    totalFrames = scene->frameCount();
    index = 0;
  }

  IteratorImplBase(nlohmann::json json, nlohmann::json layout, Resource resource)
  {
    BuilderResult result;
    initInternal(std::move(json), std::move(layout), std::move(resource), ExportOption(), result);
  }
  IteratorImplBase(
    nlohmann::json      json,
    nlohmann::json      layout,
    Resource            resource,
    const ExportOption& exportOpt,
    BuilderResult&      result)
  {
    initInternal(std::move(json), std::move(layout), std::move(resource), exportOpt, result);
  }
  ~IteratorImplBase() = default;
  bool tryNext()
  {
    if (index >= scene->frameCount())
    {
      return false;
    }
    auto f = scene->frame(index);
    while (f && !f->isVisible() && index < scene->frameCount())
    {
      // skip invisble frame
      index++;
      f = scene->frame(index);
    }
    if (index >= scene->frameCount())
    {
      return false;
    }
    if (!f)
      return false;
    return true;
  }

  void advance()
  {
    index++;
  }
};

class ImageIteratorImpl : public IteratorImplBase
{
public:
  static constexpr int    MAX_WIDTH = 8192;
  static constexpr int    MAX_HEIGHT = 8192;
  Exporter&               exporter;
  ImageOption::SizePolicy size;
  ImageIteratorImpl(
    Exporter&               exporter,
    nlohmann::json          json,
    nlohmann::json          layout,
    Resource                resource,
    ImageOption::SizePolicy size,
    const ExportOption&     opt,
    BuilderResult&          result)
    : IteratorImplBase(std::move(json), std::move(layout), std::move(resource), opt, result)
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
    if (!tryNext())
      return false;
    auto f = scene->frame(index);
    scene->setPage(index);
    f->revalidate();
    const auto b = f->bound();
    const auto id = f->guid();
    const auto w = b.size().x;
    const auto h = b.size().y;

    auto          state = exporter.d_impl.get();
    float         actualSize[2];
    float         scale = 1.0;
    constexpr int MAX_SIDE = std::min(MAX_WIDTH, MAX_HEIGHT);
    std::visit(
      Overloaded{
        [&](const ImageOption::ScaleDetermine& s)
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
      },
      size);
    layer::ImageOptions opts;
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
    auto res = state->render(scene, scale, opts, cost);
    if (!res.has_value())
    {
      return false;
    }
    key = std::move(id);
    image = std::move(res.value());
    advance();
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
  Resource            resource,
  const ImageOption&  opt,
  const ExportOption& exportOpt,
  BuilderResult&      result)
  : d_impl(std::make_unique<ImageIteratorImpl>(
      exporter,
      std::move(design),
      std::move(layout),
      std::move(resource),
      opt.size,
      exportOpt,
      result))
  , m_opts(opt)
{
}

ImageIterator::~ImageIterator() = default;

SVGIterator::SVGIterator(nlohmann::json design, nlohmann::json layout, Resource resource)
  : d_impl(
      std::make_unique<IteratorImplBase>(std::move(design), std::move(layout), std::move(resource)))
{
}

SVGIterator::SVGIterator(
  nlohmann::json      design,
  nlohmann::json      layout,
  Resource            resource,
  const ExportOption& opt,
  BuilderResult&      result)
  : d_impl(std::make_unique<IteratorImplBase>(
      std::move(design),
      std::move(layout),
      std::move(resource),
      opt,
      result))
{
}

SVGIterator::SVGIterator(SVGIterator&& other) noexcept
  : d_impl(std::move(other.d_impl))
{
}
bool SVGIterator::next(std::string& key, std::vector<char>& data)
{
  if (!d_impl->tryNext())
    return false;
  auto scene = d_impl->scene.get();
  auto f = scene->frame(d_impl->index);
  auto b = f->bound();
  auto id = f->guid();

  layer::exporter::SVGOptions opts;
  opts.extend[0] = b.width();
  opts.extend[1] = b.height();
  scene->setPage(d_impl->index);
  auto res = layer::exporter::makeSVG(scene, opts);
  if (!res.has_value())
  {
    return false;
  }
  key = std::move(id);
  data = std::move(res.value());
  d_impl->advance();
  return true;
}

IteratorResult SVGIterator::next()
{
  std::string       key;
  std::vector<char> image;
  Timer             t;
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

PDFIterator::PDFIterator(nlohmann::json design, nlohmann::json layout, Resource resource)
  : d_impl(
      std::make_unique<IteratorImplBase>(std::move(design), std::move(layout), std::move(resource)))
{
}

PDFIterator::PDFIterator(
  nlohmann::json      design,
  nlohmann::json      layout,
  Resource            resource,
  const ExportOption& opt,
  BuilderResult&      result)
  : d_impl(std::make_unique<IteratorImplBase>(
      std::move(design),
      std::move(layout),
      std::move(resource),
      opt,
      result))
{
}

PDFIterator::PDFIterator(PDFIterator&& other) noexcept
  : d_impl(std::move(other.d_impl))
{
}
bool PDFIterator::next(std::string& key, std::vector<char>& data)
{
  if (!d_impl->tryNext())
    return false;
  auto                        scene = d_impl->scene.get();
  auto                        f = scene->frame(d_impl->index);
  auto                        id = f->guid();
  auto                        b = f->bound();
  layer::exporter::PDFOptions opts;
  opts.extend[0] = b.width();
  opts.extend[1] = b.height();
  scene->setPage(d_impl->index);
  auto res = layer::exporter::makePDF(scene, opts);
  if (!res.has_value())
  {
    return false;
  }
  key = std::move(id);
  data = std::move(res.value());
  d_impl->advance();
  return true;
}

IteratorResult PDFIterator::next()
{
  std::string       key;
  std::vector<char> image;
  Timer             t;
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
