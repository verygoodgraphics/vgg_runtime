#include "ContextVk.hpp"
#include "Domain/Layout/ExpandSymbol.hpp"
#include "Utility/ConfigManager.hpp"
#include "VGG/Layer/Core/PaintNode.hpp"
#include "VGG/Layer/VGGLayer.hpp"
#include "VGG/Layer/Scene.hpp"
#include "VGG/Exporter/ImageExporter.hpp"

#include <iostream>
#include <algorithm>
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

class Exporter__pImpl
{
  Exporter* q_api; // NOLINT
public:
  std::shared_ptr<layer::GraphicsContext> ctx;
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
    auto id = scene->frame(scene->currentPage())->guid();
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
  cfg.multiSample = 0;
  d_impl->ctx->init(cfg);
  d_impl->layer->init(d_impl->ctx.get());
}

void Exporter::info(ExporterInfo* info)
{
  if (info)
  {
    info->backend.type = EBackend::VULKAN;
    info->backend.majorVersion = 1;
    info->backend.minorVersion = 3;
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

class Exporter::Iterator__pImpl
{
  Exporter::Iterator* q_api{ nullptr }; // NOLINT
public:
  Exporter& exporter;
  float maxSurfaceSize[2];
  int totalFrames{ 0 };
  int index{ 0 };
  std::shared_ptr<Scene> scene;
  Iterator__pImpl(Exporter::Iterator* api,
                  Exporter& exporter,
                  nlohmann::json json,
                  Resource resource,
                  const ImageOption& opt)
    : q_api(api)
    , exporter(exporter)
  {
    getMaxSurfaceSize(opt.resolutionLevel, maxSurfaceSize);
    scene = std::make_shared<Scene>();
    Layout::ExpandSymbol e(json);
    scene->loadFileContent(e());
    scene->setResRepo(resource);
    totalFrames = scene->frameCount();
    index = 0;
    exporter.d_impl->resize(maxSurfaceSize[0], maxSurfaceSize[1]);
  }

  bool next(std::string& key, std::vector<char>& image)
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

    const auto b = f->getBound();
    const auto id = f->guid();
    int w = b.size().x;
    int h = b.size().y;

    // f is visible
    auto state = exporter.d_impl.get();
    float actualSize[2];
    auto scale =
      calcScaleFactor(w, h, maxSurfaceSize[0], maxSurfaceSize[1], actualSize[0], actualSize[1]);
    layer::ImageOptions opts;
    opts.encode = layer::EImageEncode::IE_PNG;
    opts.position[0] = 0;
    opts.position[1] = 0;
    opts.extend[0] = actualSize[0];
    opts.extend[1] = actualSize[1];
    auto res = state->render(scene, scale, opts);
    if (!res.has_value())
    {
      return false;
    }
    key = std::move(id);
    image = std::move(res.value());

    index++;
    scene->setPage(index); // advanced page
    return true;
  }
};

bool Exporter::Iterator::next(std::string& key, std::vector<char>& image)
{
  return d_impl->next(key, image);
}

Exporter::Iterator::Iterator(Exporter::Iterator&& other) noexcept
{
  *this = std::move(other);
}
Exporter::Iterator& Exporter::Iterator::Iterator::operator=(Exporter::Iterator&& other) noexcept
{
  d_impl = std::move(other.d_impl);
  return *this;
}

Exporter::Iterator::~Iterator() = default;

Exporter::Iterator::Iterator(Exporter& exporter,
                             nlohmann::json json,
                             Resource resources,
                             const ImageOption& opt)
  : d_impl(
      new Exporter::Iterator__pImpl(this, exporter, std::move(json), std::move(resources), opt))
{
}

void render(const nlohmann::json& j,
            const std::map<std::string, std::vector<char>>& resources,
            int resolutionLevel)
{
  float maxSurfaceSize[2];
  getMaxSurfaceSize(resolutionLevel, maxSurfaceSize);

  std::shared_ptr<layer::GraphicsContext> ctx;
  ctx = std::make_shared<VkGraphicsContext>();
  layer::ContextConfig cfg;
  cfg.stencilBit = 8;
  cfg.multiSample = 0;
  ctx->init(cfg);
  Layout::ExpandSymbol e(j);
  auto scene = std::make_shared<Scene>();
  scene->loadFileContent(e());
  scene->setResRepo(resources);
  auto count = scene->frameCount();
  int threadNum = 2;

  const auto task = [&](int begin, int end)
  {
    std::stringstream ss;
    std::vector<std::pair<std::string, std::vector<char>>> res;
    auto layer = std::make_shared<layer::VLayer>();
    layer->init(ctx.get());
    layer->resize(maxSurfaceSize[0], maxSurfaceSize[1]);

    auto scene = std::make_shared<Scene>();
    scene->loadFileContent(j);
    scene->setResRepo(resources);
    layer->addScene(scene);
    for (int i = begin; i < end; i++)
    {
      auto f = scene->frame(i);
      if (!f->isVisible())
        continue;
      auto b = f->getBound();
      int w = b.size().x;
      int h = b.size().y;
      scene->setPage(i);

      float actualSize[2];
      auto scale =
        calcScaleFactor(w, h, maxSurfaceSize[0], maxSurfaceSize[1], actualSize[0], actualSize[1]);
      layer->setScaleFactor(scale);

      DEBUG("Render Canvas Size: [%d, %d]", (int)maxSurfaceSize[0], (int)maxSurfaceSize[1]);
      DEBUG("Original [%d, %d] x Scale[%f] = Final[%d, %d]",
            w,
            h,
            scale,
            (int)actualSize[0],
            (int)actualSize[1]);

      // begin render one frame
      layer->beginFrame();
      layer->render();
      layer->endFrame();
      // end render one frame

      layer::ImageOptions opts;
      opts.encode = layer::EImageEncode::IE_PNG;
      opts.position[0] = 0;
      opts.position[1] = 0;
      opts.extend[0] = actualSize[0];
      opts.extend[1] = actualSize[1];
      if (auto img = layer->makeImageSnapshot(opts); img)
      {
        // res.emplace_back(scene->frame(i)->guid(), std::move(img.value()));
        auto name = scene->frame(i)->guid();
        std::cout << "Thread: " << std::this_thread::get_id() << " ," << name << std::endl;
        std::ofstream ofs(name);
        if (ofs.is_open())
          ofs.write(img->data(), img->size());
      }
      else
      {
        ss << "Failed to encode image for artboard: " << i + 1 << std::endl;
      }
    }
    return res;
  };

  std::thread task1(task, 0, count);
  std::thread task2(task, count / 2, count);

  if (task1.joinable())
    task1.join();
  if (task2.joinable())
    task2.join();
}
}; // namespace VGG::exporter
