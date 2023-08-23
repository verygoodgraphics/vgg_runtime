#include "Event/Event.h"
#include "AppBase.hpp"
#include "TestEventListener.h"
#include <exception>
using namespace VGG;
using namespace VGG::app;
namespace fs = std::filesystem;
#define main main
int main(int argc, char** argv)
{
  AppConfig cfg;
  cfg.appName = "Renderer";
  cfg.graphicsContextConfig.drawableSize[0] = 1920;
  cfg.graphicsContextConfig.drawableSize[1] = 1080;
  cfg.graphicsContextConfig.multiSample = 0;
  cfg.graphicsContextConfig.stencilBit = 8;
  cfg.graphicsContextConfig.scaleFactor = 1.0f;
  cfg.graphicsContextConfig.resolutionScale = 1.0f;
  cfg.argc = argc;
  cfg.argv = argv;
  cfg.eventListener = std::make_unique<MyEventListener>();
  try
  {
    AppImpl::createInstance(std::move(cfg));
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
  return AppImpl::app()->exec();
}
