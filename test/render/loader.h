#pragma once
#include <Reader/IReader.hpp>
#include <filesystem>
#include <ConfigMananger.h>

namespace fs = std::filesystem;
using namespace VGG;

class ExternalToolReader : public VGG::IReader
{
public:
  virtual DataWrapper read(const fs::path& fullpath)
  {
    return readImpl(fullpath);
  }
  void setConfig(const nlohmann::json& j)
  {
    this->config = j;
  }

protected:
  DataWrapper readImpl(const fs::path& file)
  {
    const auto cmd = genCmd(file);
    auto ret = executeExternalCmd(cmd);
    DataWrapper data;
    if (ret == 0)
    {
      data.Format = readJson(getReadFile());
      data.Resource = readRes(getResource());
    }
    return data;
  }
  std::string genCmd(const fs::path& filepath)
  {
    std::string cmd = config.at("cmd");
    std::string outputImageDir = config.at("outputDir");
    return cmd + " " + filepath.string() + " " + outputImageDir;
  }
  int executeExternalCmd(const std::string& cmd)
  {
    auto back = fs::current_path();
    const std::string cwd = config.at("cwd");
    if (!cwd.empty())
    {
      fs::current_path(cwd);
    }
    auto ret = std::system(cmd.c_str());
    fs::current_path(back);
    return ret;
  }

  fs::path getReadFile()
  {
    const auto outputDir = fs::path(config.at("outputDir"));
    const auto fileName = fs::path(config.at("outputFileName"));
    return outputDir / fileName;
  }

  fs::path getResource()
  {
    const auto outputDir = fs::path(config.at("outputDir"));
    const auto image = fs::path(config.at("outputImageDir"));
    return outputDir / image;
  }

  std::map<std::string, std::vector<char>> readRes(const fs::path& fullpath)
  {
    std::map<std::string, std::vector<char>> resources;
    if (std::filesystem::exists(fullpath) == false)
      return resources;
    const fs::path prefix = this->config.value("outputImageDir", "resources");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(fullpath))
    {
      std::string key = (prefix / entry.path().filename()).string();
      std::cout << "read image: " << entry.path() << " which key is: " << key << std::endl;
      auto data = GetBinFromFile(entry.path());
      if (data.has_value())
      {
        if (data.value().empty())
        {
          std::cout << "Image is empty: " << key << std::endl;
        }
        else
        {
          resources[key] = data.value();
        }
      }
      else
      {
        std::cout << "Failed to read image: " << key << std::endl;
      }
    }
    return resources;
  }
  nlohmann::json readJson(const fs::path& fullpath) const
  {
    std::ifstream fs(fullpath);
    nlohmann::json json;
    if (fs.is_open())
    {
      fs >> json;
    }
    return json;
  }
};

class AiFileReader : public ExternalToolReader
{
};

class FigmaFileReader : public ExternalToolReader
{
};

class RawFileReader : public ExternalToolReader
{
public:
  RawFileReader()
  {
  }
  DataWrapper read(const fs::path& fullpath) override
  {
    DataWrapper data;
    data.Format = readJson(fullpath);
    const fs::path prefix = this->config.value("outputImageDir", "resources");
    auto stem = fullpath.parent_path() / prefix;
    data.Resource = readRes(stem);
    return data;
  }
};

class SketchFileReader : public ExternalToolReader
{
};

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
    r->setConfig(cfg.at("rawParser"));
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
