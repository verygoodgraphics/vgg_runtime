#include "viewer.hpp"
#include "Layer/Graphics/GraphicsSkia.hpp"
#include "Layer/Graphics/VSkiaGL.hpp"
#include "Layer/RasterManager.hpp"
#include "Layer/SimpleRasterExecutor.hpp"
#include "loop.hpp"

#ifdef IMGUI_ENABLED
#include "imgui_integration.hpp"
#include "imguipanel.hpp"
#endif

#include "Layer/GlobalSettings.hpp"
#include "Layer/Core/RasterNode.hpp"
#include "Layer/Core/VNode.hpp"
#include "Layer/Raster.hpp"
#include "Layer/Core/ZoomerNode.hpp"
#include "Layer/Core/ViewportNode.hpp"
#include "Layer/Core/EventManager.hpp"
#include "Layer/Renderer.hpp"

#include <argparse/argparse.hpp>
#include <core/SkColor.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrRecordingContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>

#include <exception>
#include <thread>
using namespace VGG;
using namespace VGG::app;
namespace fs = std::filesystem;

constexpr char POS_ARG_INPUT_FILE[] = "fig/ai/sketch/json";

VGG::layer::Ref<VGG::layer::SceneNode> loadScene(const argparse::ArgumentParser& program)
{
  std::filesystem::path prefix;
  std::filesystem::path respath;

  // VGG::layer::setAnimatedPatternEnabled(false);

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

void drawDebug(
  SkCanvas*                  canvas,
  Viewer&                    viewer,
  const layer::Revalidation& rev,
  const std::vector<Bounds>& damageBounds)
{
  using namespace VGG::layer;
#ifdef VGG_LAYER_DEBUG
  Renderer r;
  r.setCanvas(canvas);
  if (viewer.enableDebug)
  {
    viewer.rasterNode->debug(&r);
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

int run(Loop& loop, Viewer& viewer, const argparse::ArgumentParser& program)
{
  using namespace VGG::layer;
  using namespace VGG;
  layer::setupEnv();
  const auto& config = loop.getConfig();

#ifdef IMGUI_ENABLED
  initImGUI(loop.sdlState->window, loop.sdlState->glContext);
#endif

  sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface();
  sk_sp<GrRecordingContext>  directContext = skia_impl::gl::glContextCreateProc(nullptr)();

  auto surfCreateProc = skia_impl::gl::glSurfaceCreateProc();

  viewer.zoomNode = VGG::layer::ZoomerNode::Make();
  auto scale = loop.resolutionScale();
  viewer.viewportNode = VGG::layer::Viewport::Make(scale);

  sk_sp<SkSurface> gpuSurface = surfCreateProc(
    directContext.get(),
    1920 * scale,
    1080 * scale,
    { config.stencilBit, config.multiSample });
  viewer.viewportNode->setViewport(Bounds{ 0, 0, 1920 * scale, 1080 * scale });

#ifdef IMGUI_ENABLED
  Panel panel(viewer);
#endif
  loop.setEventCallback(
    [&, directContext = directContext.get()](const SDL_Event& event, void* userData)
    {
#ifdef IMGUI_ENABLED
      processEventImGUI(event);
#endif
      auto evt = toUEvent(event, loop.resolutionScale());

      if (auto& window = evt.window;
          evt.type == VGG_WINDOWEVENT &&
          (window.event == VGG_WINDOWEVENT_RESIZED || window.event == VGG_WINDOWEVENT_SIZE_CHANGED))
      {
        gpuSurface = surfCreateProc(
          directContext,
          window.drawableWidth,
          window.drawableHeight,
          { config.stencilBit, config.multiSample });
      }
#ifdef IMGUI_ENABLED
      if (evt.type == VGG_KEYDOWN && evt.key.keysym.sym == VGGK_p)
      {
        panel.setVisible(!panel.isVisible());
      }
#endif
      viewer.onEvent(evt, userData);
    });

  SceneNode*           sceneNode = nullptr;
  SimpleRasterExecutor executor(directContext.get());
  if (auto n = loadScene(program); n)
  {
    viewer.pager = std::make_unique<Pager>(n.get());
    sceneNode = n.get();
    viewer.rasterNode =
      VGG::layer::raster::make(&executor, viewer.viewportNode, viewer.zoomNode, std::move(n));
#ifdef IMGUI_ENABLED
    panel.setScene(n.get());
#endif
  }
  if (!viewer.rasterNode)
  {
    std::cout << "Cannot load scene\n";
    return 1;
  }
  constexpr float TIME_PER_FRAME = 16.67;
  while (!loop.exit())
  {

    Timer t;
    t.start();
    loop.pollEvent(0);
#ifdef IMGUI_ENABLED
    beginImGUIFrame();
    drawImGUIFrame(&panel);
#endif
    Revalidation        rev;
    std::vector<Bounds> damageBounds;
    {
      VGG::layer::EventManager::pollEvents();
      viewer.rasterNode->revalidate(&rev, glm::mat3{ 1 });
      damageBounds = mergeBounds(rev.boundsArray());
      viewer.rasterNode->raster(damageBounds);
      Renderer r;
      auto     canvas = gpuSurface->getCanvas();
      r.setCanvas(canvas);
      canvas->clear(SK_ColorWHITE);
      viewer.rasterNode->render(&r);
    }

    drawDebug(gpuSurface->getCanvas(), viewer, rev, damageBounds);

    directContext->asDirectContext()->flush();

#ifdef IMGUI_ENABLED
    endImGUIFrame();
#endif

    loop.swap();
    t.stop();
    auto ms = (float)t.duration().ms();
    if (ms > TIME_PER_FRAME)
    {
      INFO("Frame Time Cost: %f", ms);
    }
    if (ms < TIME_PER_FRAME)
      std::this_thread::sleep_for(std::chrono::milliseconds((int)(TIME_PER_FRAME - ms)));
  }

#ifdef IMGUI_ENABLED
  shutdownImGUI();
#endif
  return 0;
}

#define main main
int main(int argc, char** argv)
{
  auto         program = parse(argc, argv);
  Viewer       viewer;
  Loop::Config config{ .multiSample = 0, .stencilBit = 8 };
  Loop         loop("Viewer", 1920, 1080, config);
  loop.makeCurrent();
  return run(loop, viewer, program);
}
