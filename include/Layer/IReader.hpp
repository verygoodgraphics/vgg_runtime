#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <map>
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

struct DataWrapper
{
  nlohmann::json Format;
  std::map<std::string, std::vector<char>> Resource;
};

class IReader : public std::enable_shared_from_this<IReader>
{
protected:
  nlohmann::json config;

public:
  virtual DataWrapper read(const fs::path& fullpath) = 0;
  void setConfig(const nlohmann::json& j)
  {
    this->config = j;
  }
};

} // namespace VGG