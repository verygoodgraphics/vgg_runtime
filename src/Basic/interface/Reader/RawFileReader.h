#pragma once

#include "Common/Config.h"
#include "IReader.hpp"

#include <iostream>
#include <string>
#include <filesystem>

namespace VGG
{

class VGG_EXPORTS RawFileReader : public IReader
{
  std::filesystem::path jsonFilename;
  std::filesystem::path resDir;

public:
  RawFileReader(const std::string& filename, const std::string& resDir)
    : jsonFilename(filename)
    , resDir(resDir)
  {
  }
  nlohmann::json readFormat() override
  {
    return nlohmann::json::parse(GetTextFromFile(prefix / jsonFilename).value_or(""));
  }

  std::map<std::string, std::vector<char>> readResource() override
  {
    std::map<std::string, std::vector<char>> resources;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(prefix / resDir))
    {
      std::string key = resDir / entry.path().filename().string();
      std::cout << "read image: " << entry.path() << " which key is " << key << std::endl;
      resources[key] = GetBinFromFile(entry.path()).value_or(std::vector<char>{});
    }
    return resources;
  }
};
} // namespace VGG
