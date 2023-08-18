#include "Event/Event.h"
#include "TestEventListener.h"
using namespace VGG;
namespace fs = std::filesystem;
#define main main
int main(int argc, char** argv)
{
  auto app = AppImpl::getInstance(1920, 1080, "VGG");
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
  while (!app->shouldExit())
  {
    app->exec();
  }
  return 0;
}
