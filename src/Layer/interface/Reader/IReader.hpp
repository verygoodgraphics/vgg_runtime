#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>

#include <iostream>

namespace VGG
{

namespace fs = std::filesystem;

inline std::optional<std::string> GetTextFromFile(const std::filesystem::path fileName)
{
  std::ifstream in(fileName, std::ios::in);
  std::cout << "Full path: " << fileName << "\n";
  if (in.is_open() == false)
  {
    return std::nullopt;
  }
  return std::string{ std::istreambuf_iterator<char>{ in }, std::istreambuf_iterator<char>{} };
}

inline std::optional<std::vector<char>> GetBinFromFile(const std::filesystem::path fileName)
{
  std::ifstream in(fileName, std::ios::binary);
  std::cout << "Full path: " << fileName << "\n";
  if (in.is_open() == false)
  {
    return std::nullopt;
  }
  return std::vector<char>{ std::istreambuf_iterator<char>{ in },
                            std::istreambuf_iterator<char>{} };
}

struct Data
{
  nlohmann::json Format;
  std::map<std::string, std::vector<char>> Resource;
};

class IReader : public std::enable_shared_from_this<IReader>
{
protected:
  nlohmann::json config;

public:
  virtual Data read(const fs::path& fullpath)
  {
    return readImpl(fullpath);
  }
  void setConfig(const nlohmann::json& j)
  {
    this->config = j;
  }

protected:
  Data readImpl(const fs::path& file)
  {
    const auto cmd = genCmd(file);
    auto ret = executeExternalCmd(cmd);
    Data data;
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
    auto ret = std::system(cmd.c_str());
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
    auto parentPath = fullpath.filename();
    for (const auto& entry : std::filesystem::recursive_directory_iterator(fullpath))
    {
      std::string key = (parentPath / entry.path().filename()).string();
      std::cout << "read image: " << entry.path() << " which key is " << key << std::endl;
      resources[key] = GetBinFromFile(entry.path()).value_or(std::vector<char>{});
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

} // namespace VGG
