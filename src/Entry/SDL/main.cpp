#include "Application/MainComposer.hpp"
#include "Adapter/NativeComposer.hpp"

#include "SDLRuntime.hpp"
#include <memory>
#include <argparse/argparse.hpp>
#include <filesystem>

#include <ConfigMananger.h>
#include <Scene/Scene.h>

constexpr auto DARUMA_FILE_OR_DIRECTORY = "daruma";

using namespace VGG;
#define main main
int main(int argc, char** argv)
{
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

#ifdef NDEBUG
  MainComposer main_composer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js") };
#else
  MainComposer main_composer{ new NativeComposer("https://s5.vgg.cool/vgg-sdk.esm.js", false) };
#endif

  SDLRuntime* app = App<SDLRuntime>::getInstance(1920, 1080, "VGG");
  ASSERT(app);

  app->setController(main_composer.controller());
  app->setView(main_composer.view());

  auto darumaFileOrDir = program.get<std::string>(DARUMA_FILE_OR_DIRECTORY);
  main_composer.controller()->start(darumaFileOrDir,
                                    "../asset/vgg-format.json",
                                    "../asset/vgg_layout.json");

  // if (auto file_to_edit = program.present("-e"))
  // {
  //   main_composer.enableEdit();

  //   auto file_path = file_to_edit.value();
  //   main_composer.controller()->edit(file_path);
  // }

  // enter loop
  constexpr int fps = 60;
  while (!app->shouldExit())
  {
    app->frame(fps);
    main_composer.runLoop()->dispatch();
  }
  return 0;
}
