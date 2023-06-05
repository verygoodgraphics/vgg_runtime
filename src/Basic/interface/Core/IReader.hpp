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

class IReader : public std::enable_shared_from_this<IReader>
{
protected:
  std::filesystem::path prefix;

public:
  virtual nlohmann::json readFormat() = 0;
  virtual std::map<std::string, std::vector<char>> readResource() = 0;
  void setPrefix(const std::filesystem::path& prefix)
  {
    this->prefix = prefix;
  }
};

} // namespace VGG
