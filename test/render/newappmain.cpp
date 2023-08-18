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
  while (app->shouldExit())
  {
    app->exec();
  }
  return 0;
}
