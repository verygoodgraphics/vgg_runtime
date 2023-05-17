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
#include <argparse/argparse.hpp>
#include <filesystem>
#include "Basic/IReader.hpp"
#include "Basic/ReaderFactory.h"
#include "Entry/Common/SDLRuntime.hpp"
#include "Utils/FileManager.hpp"
#include "Basic/Scene.hpp"
#include <memory>
// #include <vgg_sketch_parser/src/analyze_sketch_file/analyze_sketch_file.h>
#include <Basic/SketchFileReader.h>
#include <Basic/RawFileReader.h>

using namespace VGG;

#ifdef EMSCRIPTEN
extern "C"
{
  void emscripten_frame()
  {
    static SDLRuntime* app = App<SDLRuntime>::getInstance();
    ASSERT(app);
    constexpr int fps = 60;
    app->frame(fps);
  }
  void emscripten_main(int width, int height)
  {
    SDLRuntime* app = App<SDLRuntime>::getInstance(width, height);
    ASSERT(app);
    if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
    {
      FileManager::newFile();
    }
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

  auto scene = std::make_shared<Scene>();
  std::map<std::string, std::vector<char>> resources;
  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    auto ext = FileManager::getLoweredFileExt(fp);
    std::shared_ptr<IReader> reader;

    if (ext == "sketch")
    {
      reader = GetSketchReader(fp);
      // legacy renderer
      if (!FileManager::loadFile(fp))
      {
        FAIL("Failed to load file: %s", fp.c_str());
      }
    }
    else if (ext == "json")
    {
      std::string resFile = std::filesystem::path(fp).stem(); // same with filename as default
      if (auto res = program.present("-d"))
      {
        resFile = res.value();
      }
      reader = GetRawReader(fp, resFile);
    }

    if (reader)
    {
      nlohmann::json json = reader->readFormat();
      auto res = reader->readResource();
      Scene::setResRepo(res);
      scene->LoadFileContent(json);
    }
    else
    {
      INFO("Failed to initialize a reader\n");
    }
  }

  SDLRuntime* app = App<SDLRuntime>::getInstance(1200, 800, "VGG");
  app->setScene(scene);
  ASSERT(app);

  if (auto fm = FileManager::getInstance(); fm && fm->fileCount() < 1)
  {
    FileManager::newFile();
  }

  // enter run mode
  app->setOnFrameOnce([app]() { app->startRunMode(); });

  // enter loop
  constexpr int fps = 60;
  while (!app->shouldExit())
  {
    app->frame(fps);
  }
  return 0;
}
#endif
