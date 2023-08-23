#pragma once
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <argparse/argparse.hpp>
#include <filesystem>

#include <Scene/Scene.h>
#include <ConfigMananger.h>
#include "Application/AppRender.h"
#include "Event/Event.h"
#include "Event/EventListener.h"
#include "Event/Keycode.h"
#include "Scene/GraphicsLayer.h"
#include "Scene/VGGLayer.h"
#include "loader.h"
#include <filesystem>
#include "Entry/SDL/SDLImpl/AppSDLImpl.hpp"

using AppImpl = VGG::entry::AppSDLImpl;
namespace fs = std::filesystem;
class MyEventListener : public EventListener
{
  AppRender* m_layer{ nullptr };
  std::shared_ptr<AppScene> m_scene;

protected:
  void onAppInit(AppImpl* app, char** argv, int argc)
  {
    INFO("onAppInit");
    m_layer = app->layer();
    argparse::ArgumentParser program("vgg", "0.1");
    program.add_argument("-l", "--load").help("load from vgg or sketch file");
    program.add_argument("-d", "--data").help("resources dir");
    program.add_argument("-p", "--prefix").help("the prefix of filename or dir");
    program.add_argument("-L", "--loaddir").help("iterates all the files in the given dir");
    program.add_argument("-c", "--config").help("specify config file");
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
    if (auto configfile = program.present("-c"))
    {
      auto file = configfile.value();
      Config::readGlobalConfig(file);
    }

    m_scene = std::make_shared<AppScene>();
    m_scene->setZoomerListener(std::make_shared<AppZoomer>());

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
      auto ext = fs::path(fp).extension().string();
      if (ext == ".json")
      {
        respath = std::filesystem::path(fp).stem(); // same with filename as default
        if (auto res = program.present("-d"))
        {
          respath = res.value();
        }
      }

      auto r = load(ext);
      if (r)
      {
        auto data = r->read(prefix / fp);
        Scene::setResRepo(data.Resource);
        try
        {
          m_scene->loadFileContent(data.Format);
          m_layer->addAppScene(m_scene);
          m_layer->setDrawPositionEnabled(true);
        }
        catch (std::exception& e)
        {
          FAIL("load json failed: %s", e.what());
        }
      }
    }
  }

  void onAppExit()
  {
    INFO("Exit...");
  }

public:
  bool onEvent(UEvent evt, void* userData) override
  {
    if (evt.type == VGG_APP_INIT)
    {
      auto app = reinterpret_cast<AppImpl*>(userData);
      onAppInit(app, evt.init.argv, evt.init.argc);
      return true;
    }
    if (evt.type != VGG_KEYDOWN)
    {
      return false;
    }
    auto type = evt.type;
    auto key = evt.key.keysym.sym;
    auto mod = evt.key.keysym.mod;

    if (key == VGGK_PAGEUP && (mod & VGG_KMOD_CTRL))
    {
      INFO("Previous page");
      if (m_scene)
      {
        m_scene->preArtboard();
      }
      return true;
    }

    if (key == VGGK_PAGEDOWN && (mod & VGG_KMOD_CTRL))
    {
      INFO("Next page");
      if (m_scene)
      {
        m_scene->nextArtboard();
      }
      return true;
    }

    if (key == VGGK_c)
    {
      INFO("Capture");
      return true;
    }

    if (key == VGGK_b)
    {
      INFO("Toggle object bounding box");
      Scene::enableDrawDebugBound(!Scene::isEnableDrawDebugBound());
      return true;
    }

    if (key == VGGK_1)
    {
      INFO("Toggle cursor position");
      m_layer->setDrawPositionEnabled(!m_layer->enableDrawPosition());
      return true;
    }

    if (key == VGGK_s)
    {
      return true;
    }
    return false;
  }
};
