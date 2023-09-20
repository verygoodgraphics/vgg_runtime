#include "SDLImpl/AppSDLImpl.hpp"
#include "SDLImpl/SdlMouse.hpp"

#include "Adapter/NativeComposer.hpp"
#include "Application/AppBase.hpp"
#include "Application/MainComposer.hpp"
#include "Application/Mouse.hpp"
#include "Application/UIApplication.hpp"
#include "Utility/ConfigMananger.h"
#include "Scene/Scene.h"

#include <argparse/argparse.hpp>

#include <memory>
#include <filesystem>

constexpr auto DARUMA_FILE_OR_DIRECTORY = "daruma";

using namespace VGG;
using namespace VGG::app;

using AppImpl = VGG::entry::AppSDLImpl;

int main(int argc, char** argv)
{
  INFO("main");

  argparse::ArgumentParser program("vgg", "0.1");
  program.add_argument(DARUMA_FILE_OR_DIRECTORY).help("daruma file or directory");
  program.add_argument("-c", "--config").help("specify config file");

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

  AppConfig cfg;
  cfg.appName = "SdlRuntime";
  cfg.windowSize[0] = 1920;
  cfg.windowSize[1] = 1080;
  cfg.graphicsContextConfig.multiSample = 0;
  cfg.graphicsContextConfig.stencilBit = 8;
  cfg.argc = argc;
  cfg.argv = argv;

  auto app = new UIApplication;
  cfg.eventListener.reset(app);

  try
  {
    AppImpl::createInstance(std::move(cfg));
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  bool catchJsException = false;
#ifdef NDEBUG
  catchJsException = true;
#endif
  MainComposer mainComposer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js",
                                                catchJsException),
                             std::make_shared<SdlMouse>() };

  auto sdlApp = AppImpl::app();

  // inject dependencies
  app->setLayer(sdlApp->layer()); // 1. must be first
  app->setView(mainComposer.view());
  app->setController(mainComposer.controller());

  auto darumaFileOrDir = program.get<std::string>(DARUMA_FILE_OR_DIRECTORY);
  mainComposer.controller()->start(darumaFileOrDir,
                                   "../asset/vgg-format.json",
                                   "../asset/vgg_layout.json");

  while (!sdlApp->shouldExit())
  {
    sdlApp->poll();
    app->run(cfg.renderFPSLimit);
    mainComposer.runLoop()->dispatch();
  }

  return 0;
}
