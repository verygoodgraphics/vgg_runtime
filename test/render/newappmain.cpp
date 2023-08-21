#include "Event/Event.h"
#include "NewApp.hpp"
#include "TestEventListener.h"
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
  auto app = AppImpl::getInstance(cfg);
  if (!app)
  {
    return 1;
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
