#include "Application/Event/Event.h"
#include "Application/AppBase.hpp"
#include "TestEventListener.h"
#include "Entry/SDL/SDLImpl/AppSDLImpl.hpp"
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

#define main main
int main(int argc, char** argv)
{
  AppConfig cfg;
  cfg.appName = "Renderer";
  cfg.windowSize[0] = 1920;
  cfg.windowSize[1] = 1080;
  cfg.graphicsContextConfig.multiSample = 0;
  cfg.graphicsContextConfig.stencilBit = 8;
  cfg.argc = argc;
  cfg.argv = argv;
  cfg.eventListener = std::make_unique<MyEventListener<App>>();
  try
  {
    App::createInstance(std::move(cfg));
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  // or:
  // SDLRuntime* app = AppBase::app();
  // while (!app->shouldExit())
  // {
  //   app->poll();
  //   app->process();
  // }
  return App::app()->exec();
}
