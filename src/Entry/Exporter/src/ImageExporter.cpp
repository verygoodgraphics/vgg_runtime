#include <iostream>
#include "Domain/include/Domain/Layout/ExpandSymbol.hpp"
#include "ImageExporter.hpp"
#include "ContextVk.hpp"
#include <Scene/VGGLayer.h>
#include <Scene/Scene.h>
#include <Core/FontManager.h>
#include <Core/PaintNode.h>
#include <Utility/interface/ConfigMananger.h>
#include <algorithm>
#include <exception>
#include <memory>
#include <optional>
#include <fstream>
#include <string>
#include <sstream>

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
std::tuple<std::string, std::vector<std::pair<std::string, std::vector<char>>>> render(
  const nlohmann::json& j,
  const std::map<std::string, std::vector<char>>& resources,
  int imageQuality,
  int resolutionLevel,
  const std::string& configFile,
  const std::string& fontCollectionName)
{

  Config::readGlobalConfig(configFile);
  float maxSurfaceSize[2];
  getMaxSurfaceSize(resolutionLevel, maxSurfaceSize);
  std::stringstream ss;

  std::shared_ptr<layer::GraphicsContext> ctx;
  ctx = std::make_shared<VkGraphicsContext>();
  auto layer = std::make_shared<layer::VLayer>();
  layer::ContextConfig cfg;
  cfg.windowSize[0] = maxSurfaceSize[0];
  cfg.windowSize[1] = maxSurfaceSize[1];
  cfg.stencilBit = 8;
  cfg.multiSample = 0;
  ctx->init(cfg);
  layer->init(ctx.get());

  FontManager::instance().setDefaultFontManager(fontCollectionName);
  std::vector<std::pair<std::string, std::vector<char>>> res;
  auto scene = std::make_shared<Scene>();
  Layout::ExpandSymbol e(j);
  scene->loadFileContent(e());
  scene->setResRepo(resources);
  layer->addScene(scene);
  auto count = scene->frameCount();
  for (int i = 0; i < count; i++)
  {
    auto b = scene->frame(i)->getBound();
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
      res.emplace_back(scene->frame(i)->guid(), std::move(img.value()));
    }
    else
    {
      ss << "Failed to encode image for artboard: " << i + 1 << std::endl;
    }
  }
  // layer->shutdown();
  // ctx->shutdown();
  return { std::string{ std::istreambuf_iterator<char>{ ss }, std::istreambuf_iterator<char>{} },
           res };
}
}; // namespace VGG::exporter
