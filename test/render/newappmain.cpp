#include "Event/Event.h"
#include "NewApp.hpp"
#include "TestEventListener.h"
#include <exception>
using namespace VGG;
namespace fs = std::filesystem;
#define main main
int main(int argc, char** argv)
{
  AppConfig cfg;
  cfg.appName = "Renderer";
  cfg.windowSize[0] = 1920;
  cfg.windowSize[1] = 1080;
  cfg.videoConfig.multiSample = 0;
  cfg.videoConfig.stencilBit = 8;
  SDLRuntime* app = nullptr;
  try
  {
    app = &AppBase::createInstance(cfg);
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
  app->setEventListener(std::make_unique<MyEventListener>());
  UEvent evt;
  evt.type = VGG_APP_INIT;
  evt.init.argc = argc;
  evt.init.argv = argv;
  app->sendEvent(evt);

  // or:
  // while (!app->shouldExit())
  // {
  //   app->poll();
  //   app->process();
  // }
  return app->exec();
}
