/*
 * Copyright (C) 2021-2023 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Main/MainComposer.hpp"

#include <argparse/argparse.hpp>
#include <filesystem>
#include "Entry/SDL/SDLRuntime.hpp"
#include "Utils/FileManager.hpp"
#include <memory>

#include <Scene/Scene.h>

using namespace VGG;
namespace fs = std::filesystem;

#ifdef EMSCRIPTEN
extern "C"
{
  void emscripten_frame()
  {
    static SDLRuntime* app = App<SDLRuntime>::getInstance();
    ASSERT(app);
    constexpr int fps = 60;
    app->frame(fps);

    auto& main_composer = MainComposer::instance();
    main_composer.runLoop()->dispatch();
  }
  void emscripten_main(int width, int height)
  {
    SDLRuntime* app = App<SDLRuntime>::getInstance(width, height);
    ASSERT(app);
    if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
    {
      FileManager::newFile();
    }

    auto& main_composer = MainComposer::instance();
    app->setView(main_composer.view());
    app->setScene(main_composer.view()->scene());

    app->setOnFrameOnce([app]() { app->startRunMode(); });
    emscripten_set_main_loop(emscripten_frame, 0, 1);
  }
}
#else
int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
  program.add_argument("-l", "--load").help("load from vgg or sketch file");
  program.add_argument("-d", "--data").help("resources dir");
  program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
  program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");

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

  MainComposer main_composer;

  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();

    // todo, add json schema
    main_composer.controller()->start(fp);
  }

  SDLRuntime* app = App<SDLRuntime>::getInstance(1200, 800, "VGG");
  ASSERT(app);

  // todo: delete deprecated code
  if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
  {
    FileManager::newFile();
  }

  app->setView(main_composer.view());
  app->setScene(main_composer.view()->scene());

  // enter run mode
  app->setOnFrameOnce([app]() { app->startRunMode(); });

  // enter loop
  constexpr int fps = 60;
  while (!app->shouldExit())
  {
    app->frame(fps);
    main_composer.runLoop()->dispatch();
  }
  return 0;
}
#endif
