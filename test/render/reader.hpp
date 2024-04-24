#pragma once

#include "Layer/Core/ResourceProvider.hpp"
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

inline std::optional<std::string> readText(const std::filesystem::path fileName)
{
  std::ifstream in(fileName, std::ios::in);
  if (in.is_open() == false)
  {
    return std::nullopt;
  }
  return std::string{ std::istreambuf_iterator<char>{ in }, std::istreambuf_iterator<char>{} };
}

inline std::optional<std::vector<char>> readBinary(const std::filesystem::path fileName)
{
  std::ifstream in(fileName, std::ios::binary);
  if (in.is_open() == false)
  {
    return std::nullopt;
  }
  return std::vector<char>{ std::istreambuf_iterator<char>{ in },
                            std::istreambuf_iterator<char>{} };
}

struct DataWrapper
{
  nlohmann::json                           format;
  nlohmann::json                           layout;
  std::unique_ptr<layer::ResourceProvider> provider;
};

class IReader : public std::enable_shared_from_this<IReader>
{
protected:
  nlohmann::json m_config;

public:
  virtual DataWrapper read(const fs::path& fullpath) = 0;
  void                setConfig(const nlohmann::json& j)
  {
    this->m_config = j;
  }
};

} // namespace VGG
