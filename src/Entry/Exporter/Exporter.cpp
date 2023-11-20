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
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Utility/ConfigManager.hpp"
#include "Layer/Core/PaintNode.hpp"
#include "Layer/VGGLayer.hpp"
#include "Layer/Scene.hpp"
#include "Layer/Exporter/SVGExporter.hpp"
#include "Layer/Exporter/PDFExporter.hpp"

#include "VGG/Exporter/ImageExporter.hpp"
#include "VGG/Exporter/SVGExporter.hpp"
#include "VGG/Exporter/PDFExporter.hpp"

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
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
    break;
    case 1:

    {
      maxSurfaceSize[0] = 2048;
      maxSurfaceSize[1] = 2048;
    }
    break;
    case 2:
    {
      maxSurfaceSize[0] = 4096;
      maxSurfaceSize[1] = 4096;
    }
    break;
    case 3:
    {
      maxSurfaceSize[0] = 8192;
      maxSurfaceSize[1] = 8192;
    }
    break;
    case 4:
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

float calcScaleFactor(float inputWidth,
                      float inputHeight,
                      float maxWidth,
                      float maxHeight,
                      float& outWidth,
                      float& outHeight)
{
  auto widthScale = maxWidth / inputWidth;
  auto heightScale = maxHeight / inputHeight;
  float outputSize[2] = { 0.f, 0.f };
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
  std::shared_ptr<layer::VLayer> layer;
  std::shared_ptr<Scene> scene;
  OutputCallback outputCallback;
  Exporter__pImpl(Exporter* api)
    : q_api(api)
  {
  }

  void resize(int w, int h)
  {
    layer->resize(w, h);
  }

  std::optional<std::vector<char>> render(std::shared_ptr<Scene> scene,
                                          float scale,
                                          const layer::ImageOptions& opts)
  {
    // auto id = scene->frame(scene->currentPage())->guid();
    layer->setScene(std::move(scene));
    layer->setScaleFactor(scale);

    // begin render one frame
    layer->beginFrame();
    layer->render();
    layer->endFrame();
    // end render one frame

    return layer->makeImageSnapshot(opts);
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
  float maxSurfaceSize[2];
  int totalFrames{ 0 };
  int index{ 0 };
  std::shared_ptr<Scene> scene;

  IteratorImplBase(nlohmann::json json, nlohmann::json layout, Resource resource)
  {
    scene = std::make_shared<Scene>();
    Layout::ExpandSymbol e(std::move(json), std::move(layout));
    scene->loadFileContent(e());
    scene->setResRepo(std::move(resource));
    totalFrames = scene->frameCount();
    index = 0;
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
  Exporter& exporter;
  ImageIteratorImpl(Exporter& exporter,
                    nlohmann::json json,
                    nlohmann::json layout,
                    Resource resource,
                    int resolutionLevel)
    : IteratorImplBase(std::move(json), std::move(layout), std::move(resource))
    , exporter(exporter)
  {
    getMaxSurfaceSize(resolutionLevel, maxSurfaceSize);
    exporter.d_impl->resize(maxSurfaceSize[0], maxSurfaceSize[1]);
  }

  bool next(std::string& key, std::vector<char>& image, EImageType type, int quality)
  {
    if (!tryNext())
      return false;

    auto f = scene->frame(index);
    scene->setPage(index);
    const auto b = f->getBound();
    const auto id = f->guid();
    int w = b.size().x;
    int h = b.size().y;

    auto state = exporter.d_impl.get();
    float actualSize[2];
    auto scale =
      calcScaleFactor(w, h, maxSurfaceSize[0], maxSurfaceSize[1], actualSize[0], actualSize[1]);
    layer::ImageOptions opts;
    opts.encode = toEImageEncode(type);
    opts.position[0] = 0;
    opts.position[1] = 0;
    opts.extend[0] = actualSize[0];
    opts.extend[1] = actualSize[1];
    opts.quality = quality;
    auto res = state->render(scene, scale, opts);
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
  return d_impl->next(key, image, m_opts.type, m_opts.imageQuality);
}

ImageIterator::ImageIterator(ImageIterator&& other) noexcept
  : m_opts(other.m_opts)
  , d_impl(std::move(other.d_impl))
{
}
ImageIterator::ImageIterator(Exporter& exporter,
                             nlohmann::json design,
                             nlohmann::json layout,
                             Resource resource,
                             const ImageOption& opt)
  : d_impl(std::make_unique<ImageIteratorImpl>(exporter,
                                               std::move(design),
                                               std::move(layout),
                                               std::move(resource),
                                               opt.resolutionLevel))
  , m_opts(opt)
{
}

ImageIterator::~ImageIterator() = default;

SVGIterator::SVGIterator(nlohmann::json design, nlohmann::json layout, Resource resource)
  : d_impl(
      std::make_unique<IteratorImplBase>(std::move(design), std::move(layout), std::move(resource)))
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
  auto b = f->getBound();
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
SVGIterator::~SVGIterator() = default;

PDFIterator::PDFIterator(nlohmann::json design, nlohmann::json layout, Resource resource)
  : d_impl(
      std::make_unique<IteratorImplBase>(std::move(design), std::move(layout), std::move(resource)))
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
  auto scene = d_impl->scene.get();
  auto f = scene->frame(d_impl->index);
  auto id = f->guid();
  auto b = f->getBound();
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

PDFIterator::~PDFIterator() = default;
}; // namespace VGG::exporter
