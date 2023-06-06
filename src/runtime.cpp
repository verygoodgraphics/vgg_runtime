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
#include "Entry/Common/SDLRuntime.hpp"
#include "Utils/FileManager.hpp"
#include <memory>

#include <Scene/Scene.h>
#include <Reader/LoadUtil.hpp>

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

  auto scene = std::make_shared<Scene>();
  std::map<std::string, std::vector<char>> resources;
  std::filesystem::path prefix;
  std::filesystem::path respath;

  if (auto p = program.present("-p"))
  {
    prefix = p.value();
  }
  if (auto loadfile = program.present("-l"))
  {
    auto fp = loadfile.value();
    auto ext = FileManager::getLoweredFileExt(fp);
    if (ext == "json")
    {
      respath = std::filesystem::path(fp).stem(); // same with filename as default
      if (auto res = program.present("-d"))
      {
        respath = res.value();
      }
    }

    load(fp,
         respath,
         prefix,
         [&](const auto& json, auto res)
         {
           Scene::setResRepo(res);
           scene->loadFileContent(json);
         });

    // legacy renderer
    if (!FileManager::loadFile(prefix / fp))
    {
      FAIL("Failed to load file: %s", fp.c_str());
    }
  }

  SDLRuntime* app = App<SDLRuntime>::getInstance(1200, 800, "VGG");
  app->setScene(scene);

  std::vector<fs::path> entires;
  std::vector<fs::path>::iterator fileIter;
  if (auto L = program.present("-L"))
  {
    for (const auto& ent : fs::recursive_directory_iterator(prefix / L.value()))
    {
      entires.emplace_back(ent);
    }
    int fileIter = 0;
    app->setReloadCallback(
      [&](Scene* scene, int type)
      {
        if (type == 0 && fileIter < entires.size())
          fileIter++;
        else if (type == 1 && fileIter > 0)
          fileIter--;

        if (fileIter >= 0 && fileIter < entires.size())
        {
          load(entires[fileIter],
               {},
               {},
               [&](const auto& json, auto res)
               {
                 Scene::setResRepo(res);
                 scene->loadFileContent(json);
               });
          INFO("Open %s", entires[fileIter].string().c_str());
        }
      });
  }
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
