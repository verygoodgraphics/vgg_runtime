#include "SDLRuntime.hpp"
#include "Utils/FileManager.hpp"
#include <memory>
#include <argparse/argparse.hpp>
#include <filesystem>

#include <Scene/Scene.h>
#include <Reader/LoadUtil.hpp>

using namespace VGG;
namespace fs = std::filesystem;
#define main main
int main(int argc, char** argv)
{
  argparse::ArgumentParser program("vgg", Version::get());
  program.add_argument("-l", "--load").help("load from vgg or sketch file");
  program.add_argument("-d", "--data").help("resources dir");
  program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
  program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");
  program.add_argument("-w", "--width")
    .help("width of viewport")
    .scan<'i', int>()
    .default_value(1200);
  program.add_argument("-h", "--height")
    .help("height of viewport")
    .scan<'i', int>()
    .default_value(800);

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
    if (!FileManager::loadFile((prefix / fp).string()))
    {
      FAIL("Failed to load file: %s", fp.c_str());
    }
  }

  SDLRuntime* app = App<SDLRuntime>::getInstance(1920, 1080, "VGG");
  if (!app)
  {
    return 0;
  }
  app->setScene(scene);
  app->setDrawInfo(true);

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
