#include "Application/MainComposer.hpp"
#include "Adapter/NativeComposer.hpp"

#include "SDLRuntime.hpp"
#include <memory>
#include <argparse/argparse.hpp>
#include <filesystem>

#include <ConfigMananger.h>
#include <Scene/Scene.h>

using namespace VGG;
#define main main
int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", "0.1");
  program.add_argument("-l", "--load").help("load from daruma file or directory");
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

#ifdef NDEBUG
  MainComposer main_composer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js") };
#else
  MainComposer main_composer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js", false) };
#endif

  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    main_composer.controller()->start(fp, "../asset/vgg-format.json");
  }

  SDLRuntime* app = App<SDLRuntime>::getInstance(1920, 1080, "VGG");
  ASSERT(app);

  app->setView(main_composer.view());
  app->setScene(main_composer.view()->scene());

  // enter loop
  constexpr int fps = 60;
  while (!app->shouldExit())
  {
    app->frame(fps);
    main_composer.runLoop()->dispatch();
  }
  return 0;
}
