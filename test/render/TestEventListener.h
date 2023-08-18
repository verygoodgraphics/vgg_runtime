#pragma once
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <argparse/argparse.hpp>
#include <filesystem>

#include <Scene/Scene.h>
#include <ConfigMananger.h>
#include "Event/Event.h"
#include "Event/EventListener.h"
#include "Scene/VGGLayer.h"
#include "Entry/SDL/NewSDLRuntime.hpp"

using AppImpl = App<NewSDLRuntime>;
class MyEventListener : public EventListener
{
  VLayer* m_layer{ nullptr };

  std::shared_ptr<Scene> m_scene;

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
  }

  void onAppExit()
  {
    INFO("Exit...");
  }

public:
  bool dispatchEvent(UEvent evt, void* userData) override
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

    if (key == VGGK_PAGEUP && (mod & KMOD_CTRL))
    {
      INFO("Previous page");
      if (m_scene)
      {
        m_scene->preArtboard();
      }
      return true;
    }

    if (key == VGGK_PAGEDOWN && (mod & KMOD_CTRL))
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

    if (key == VGGK_s)
    {
      return true;
    }
    return false;
  }
};
