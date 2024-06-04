#include "viewer.hpp"

#include "Application/Event/Event.hpp"
#include "Application/AppBase.hpp"
#include "Application/Event/EventAPI.hpp"
#include "Layer/Core/RasterNode.hpp"
#include "Layer/Graphics/ContextInfoGL.hpp"
#include "Layer/Graphics/GraphicsContext.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/ViewportNode.hpp"
#include "Entry/SDL/AppSDLImpl.hpp"
#include "Layer/Graphics/GraphicsSkia.hpp"
#include "Layer/Core/EventManager.hpp"
#include "Layer/Renderer.hpp"
#include "Layer/Graphics/VSkiaContext.hpp"
#include "Layer/Graphics/VSkiaGL.hpp"

#include <argparse/argparse.hpp>

#include <core/SkColor.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <src/gpu/ganesh/gl/GrGLDefines.h>

#ifdef VGG_USE_VULKAN
#include "Entry/SDL/SDLImpl/AppSDLVkImpl.hpp"
#endif
#include <exception>
using namespace VGG;
using namespace VGG::app;
namespace fs = std::filesystem;

using AppSDLImpl = VGG::entry::AppSDLImpl;
#ifdef VGG_USE_VULKAN
using AppVkImpl = VGG::entry::AppSDLVkImpl;
#endif
using App = AppSDLImpl;

constexpr char POS_ARG_INPUT_FILE[] = "fig/ai/sketch/json";

template<typename App>
class ViewerEventListener : public VGG::app::EventListener
{
public:
  bool onEvent(UEvent evt, void* userData) override
  {
    return viewer.onEvent(evt, userData);
  }
  Viewer viewer;
};

VGG::layer::Ref<VGG::layer::RenderNode> loadScene(const argparse::ArgumentParser& program)
{
  std::filesystem::path prefix;
  std::filesystem::path respath;

  if (auto p = program.present("-p"))
  {
    prefix = p.value();
  }
  if (auto loadfile = program.present(POS_ARG_INPUT_FILE))
  {
    auto fp = loadfile.value();
    auto ext = fs::path(fp).extension().string();
    if (ext == ".json")
    {
      respath = std::filesystem::path(fp).stem(); // same with filename as default
      if (auto res = program.present("-d"))
      {
        respath = res.value();
      }
    }

    auto r = load(ext);
    if (r)
    {
      auto data = r->read(prefix / fp);

      VGG::layer::setGlobalResourceProvider(std::move(data.provider));
      try
      {
        auto res = VGG::entry::DocBuilder::builder()
                     .setDocument(std::move(data.format))
                     .setLayout(std::move(data.layout))
                     .setExpandEnabled(true)
                     .setLayoutEnabled(true)
                     .build();

        layer::SceneBuilderResult sceneBuilderResult;
        layer::Timer              t;
        if (program.get("-m") == "json")
        {
          std::vector<layer::JSONFrameObject> frames;
          std::vector<nlohmann::json>         jsonModels;
          for (auto& e : *res.doc)
          {
            if (e->type() == VGG::Domain::Element::EType::FRAME)
            {
              auto f = static_cast<VGG::Domain::FrameElement*>(e.get());
              jsonModels.emplace_back(f->treeModel(true));
              frames.emplace_back(layer::JSONFrameObject(jsonModels.back()));
            }
          }
          t.start();
          sceneBuilderResult = VGG::layer::SceneBuilder::builder()
                                 .setResetOriginEnable(true)
                                 .setAllocator(layer::getGlobalMemoryAllocator())
                                 .build<layer::JSONModelFrame>(std::move(frames));
          t.stop();
        }
        else if (program.get("-m") == "struct")
        {
          std::vector<layer::StructFrameObject> frames;
          for (auto& f : *res.doc)
          {
            if (f->type() == VGG::Domain::Element::EType::FRAME)
            {
              frames.emplace_back(layer::StructFrameObject(f.get()));
            }
          }
          t.start();
          sceneBuilderResult = VGG::layer::SceneBuilder::builder()
                                 .setResetOriginEnable(true)
                                 .setAllocator(layer::getGlobalMemoryAllocator())
                                 .build<layer::StructModelFrame>(std::move(frames));
          t.stop();
        }
        auto dur = t.elapsed();
        INFO("Doc Expand Time Cost: %f", (double)res.timeCost.expand.s());
        INFO("Doc Layout Time Cost: %f", (double)res.timeCost.layout.s());
        INFO("Scene Build Time Cost: %f", (double)dur.s());
        if (sceneBuilderResult.type)
        {
          if (
            *sceneBuilderResult.type ==
            VGG::layer::SceneBuilderResult::EResultType::VERSION_MISMATCH)
          {
            DEBUG("Format Version mismatch:");
          }
        }
        VGG::layer::Ref<VGG::layer::SceneNode> sceneNode;
        if (sceneBuilderResult.root)
        {
          sceneNode = layer::SceneNode::Make(std::move(*sceneBuilderResult.root));
          return sceneNode;
        }
      }
      catch (std::exception& e)
      {
        FAIL("load json failed: %s", e.what());
      }
    }
  }
  return nullptr;
}

argparse::ArgumentParser parse(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", "0.1");
  program.add_argument(POS_ARG_INPUT_FILE).help("input fig/ai/sketch/json file");
  program.add_argument("-d", "--data").help("resources dir");
  program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
  program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");
  program.add_argument("-c", "--config").help("specify config file");
  program.add_argument("-m", "--model").help("model read by").default_value("struct");
  program.add_argument("-w", "--width")
    .help("width of viewport")
    .scan<'i', int>()
    .default_value(1200);
  program.add_argument("-h", "--height")
    .help("height of viewport")
    .scan<'i', int>()
    .default_value(800);

  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err)
  {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }
  if (auto configfile = program.present("-c"))
  {
    auto file = configfile.value();
    Config::readGlobalConfig(file);
  }
  return program;
}

int run(const layer::ContextConfig& cfg, Viewer& viewer, const argparse::ArgumentParser& program)
{
  using namespace VGG::layer;
  using namespace VGG::layer::skia_impl;
  using namespace VGG;
  auto app = App::app();
  app->makeCurrent(); // init GL context

  // init skia
  VGG::layer::skia_impl::SkiaContext skia(
    gl::glContextCreateProc(static_cast<ContextInfoGL*>(app->contextInfo())),
    gl::glSurfaceCreateProc(),
    cfg);

  viewer.zoomNode = VGG::layer::ZoomerNode::Make();
  viewer.viewportNode = VGG::layer::Viewport::Make(1.0);
  viewer.viewportChangeCallback = [&skia](int w, int h) { skia.resizeSurface(w, h); };

  Ref<RasterNode> rasterNode;
  if (auto n = loadScene(program); n)
  {
    rasterNode = VGG::layer::RasterNodeImpl::Make(
      skia.context(),
      viewer.viewportNode,
      viewer.zoomNode,
      std::move(n));
  }
  if (!rasterNode)
  {
    std::cout << "Cannot load scene\n";
    return 1;
  }

  // Render loop
  while (!app->shouldExit())
  {
    app->poll(); // event process

    // Rendering begin
    Revalidation        rev;
    Timer               t;
    std::vector<Bounds> damageBounds;
    t.start();
    {
      VGG::layer::EventManager::pollEvents();
      rasterNode->revalidate(&rev, glm::mat3{ 1 });
      damageBounds = mergeBounds(rev.boundsArray());
      rasterNode->raster(damageBounds);
    }

    skia.prepareFrame();

    {
      Renderer r;
      auto     canvas = skia.canvas();
      r.setCanvas(canvas);
      canvas->clear(SK_ColorWHITE);
      rasterNode->render(&r);

#ifdef VGG_LAYER_DEBUG
      if (viewer.enableDebug)
      {
        rasterNode->debug(&r);
      }
#endif

      if (viewer.enableDrawDamageBounds)
      {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(2);
        p.setColor(SK_ColorBLUE);
        for (const auto& d : rev.boundsArray())
        {
          canvas->drawRect(toSkRect(d), p);
        }

        p.setColor(SK_ColorGREEN);
        p.setStyle(SkPaint::kFill_Style);
        p.setAlphaf(0.3);
        for (const auto& d : damageBounds)
        {
          canvas->drawRect(toSkRect(d), p);
        }
      }
    }

    skia.flushAndSubmit();
    t.stop();
    auto tt = (int)t.duration().ms();
    if (tt >= 16)
    {
      INFO("render time: %d ms", tt);
    }
    app->swapBuffer();
    skia.markSwap();
  }
  return 0;
}

#define main main
int main(int argc, char** argv)
{
  auto program = parse(argc, argv);

  AppConfig cfg;
  cfg.appName = "Renderer";
  cfg.windowSize[0] = 1920;
  cfg.windowSize[1] = 1080;
  cfg.graphicsContextConfig.multiSample = 0;
  cfg.graphicsContextConfig.stencilBit = 8;
  cfg.argc = argc;
  cfg.argv = argv;

  auto  listener = std::make_unique<ViewerEventListener<App>>();
  auto& viewer = listener->viewer;
  cfg.eventListener = std::move(listener);
  try
  {
    App::createInstance(std::move(cfg));
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  return run(cfg.graphicsContextConfig, viewer, program);
}
