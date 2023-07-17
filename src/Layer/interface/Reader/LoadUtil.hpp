#include "Reader/ConfigMananger.h"
#include "Reader/FigmaFileReader.h"
#include "Reader/RawFileReader.h"
#include "Reader/SketchFileReader.h"
#include "Reader/AiFileReader.h"
#include "Scene/Scene.h"
#include "Reader/IReader.hpp"
#include <exception>
#include <functional>
#include <filesystem>
#include <map>
#include <vector>

using namespace VGG;

inline std::shared_ptr<IReader> load(const std::string ext)
{
  std::shared_ptr<IReader> reader;
  auto& cfg = Config::globalConfig();
  if (ext == ".sketch")
  {
    try
    {
      auto r = std::make_shared<SketchFileReader>();
      r->setConfig(cfg.at("sketchParser"));
      reader = r;
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
  }
  else if (ext == ".json")
  {
    auto r = std::make_shared<RawFileReader>();
    reader = r;
  }
  else if (ext == ".ai")
  {
    try
    {
      auto r = std::make_shared<AiFileReader>();
      r->setConfig(cfg.at("aiParser"));
      reader = r;
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
  }
  else if (ext == ".fig")
  {
    try
    {
      const auto aiconfig = cfg.at("figmaParser");
      auto r = std::make_shared<FigmaFileReader>();
      r->setConfig(cfg.at("figmaParser"));
      reader = r;
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
  }

  if (reader)
  {
    return reader;
  }
  else
  {
  }
  return nullptr;
}
