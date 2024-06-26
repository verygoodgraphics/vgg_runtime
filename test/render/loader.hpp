#pragma once
#include "Layer/Core/DefaultResourceProvider.hpp"
#include "Utility/ConfigManager.hpp"
#include "reader.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>

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
    this->m_config = j;
  }

protected:
  DataWrapper readImpl(const fs::path& file)
  {
    const auto  cmd = genCmd(file);
    auto        ret = executeExternalCmd(cmd);
    DataWrapper data;
    if (ret == 0)
    {
      data.format = readJson(getReadFile());
      data.layout = readJson(layoutFileName());
      data.provider = readRes(getResource());
    }
    return data;
  }
  std::string genCmd(const fs::path& filepath)
  {
    std::string cmd = m_config.value("cmd", "");
    std::string outputImageDir = m_config.value("outputDir", "");
    return cmd + " " + filepath.string() + " " + outputImageDir;
  }
  int executeExternalCmd(const std::string& cmd)
  {
    auto              back = fs::current_path();
    const std::string cwd = m_config.value("cwd", "");
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
    const auto outputDir = fs::path(m_config.value("outputDir", ""));
    const auto fileName = fs::path(m_config.value("outputFileName", ""));
    return outputDir / fileName;
  }

  fs::path layoutFileName()
  {
    const auto outputDir = fs::path(m_config.value("outputDir", ""));
    const auto fileName = fs::path(m_config.value("layoutFileName", ""));
    return outputDir / fileName;
  }

  fs::path getResource()
  {
    const auto outputDir = fs::path(m_config.value("outputDir", ""));
    const auto image = fs::path(m_config.value("outputImageDir", ""));
    return outputDir / image;
  }

  std::unique_ptr<layer::ResourceProvider> readRes(const fs::path& fullpath)
  {
    if (std::filesystem::exists(fullpath) == false)
      return nullptr;
    // const fs::path prefix = this->m_config.value("outputImageDir", "resources");
    const auto prefix = fs::path(m_config.value("outputDir", fullpath.parent_path().string()));
    return std::make_unique<layer::FileResourceProvider>(prefix);
    // for (const auto& entry : std::filesystem::recursive_directory_iterator(fullpath))
    // {
    //   std::string key = (prefix / entry.path().filename()).string();
    //   std::cout << "read image: " << entry.path() << " which key is: " << key << std::endl;
    //   auto data = readBinary(entry.path());
    //   if (data.has_value())
    //   {
    //     if (data.value().empty())
    //     {
    //       std::cout << "Image is empty: " << key << std::endl;
    //     }
    //     else
    //     {
    //       resources[key] = data.value();
    //     }
    //   }
    //   else
    //   {
    //     std::cout << "Failed to read image: " << key << std::endl;
    //   }
    // }
    // return resources;
  }
  nlohmann::json readJson(const fs::path& fullpath) const
  {
    std::ifstream  fs(fullpath);
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
    data.format = readJson(fullpath);
    const fs::path prefix = this->m_config.value("outputImageDir", "resources");
    auto           stem = fullpath.parent_path() / prefix;
    data.provider = readRes(stem);
    return data;
  }
};

class SketchFileReader : public ExternalToolReader
{
};

inline std::shared_ptr<IReader> load(const std::string ext)
{
  std::shared_ptr<IReader> reader;
  auto&                    cfg = Config::globalConfig();
  if (ext == ".sketch")
  {
    try
    {
      auto r = std::make_shared<SketchFileReader>();
      r->setConfig(cfg.value("sketchParser", nlohmann::json::object()));
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
    r->setConfig(cfg.value("rawParser", nlohmann::json::object()));
    reader = r;
  }
  else if (ext == ".ai")
  {
    try
    {
      auto r = std::make_shared<AiFileReader>();
      r->setConfig(cfg.value("aiParser", nlohmann::json::object()));
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
      auto r = std::make_shared<FigmaFileReader>();
      r->setConfig(cfg.value("figmaParser", nlohmann::json::object()));
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
