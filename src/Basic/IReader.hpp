#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>

namespace VGG
{

inline std::optional<std::string> GetTextFromFile(const std::string& fileName)
{
  std::ifstream in(fileName, std::ios::in);
  if (in.is_open() == false)
  {
    return std::nullopt;
  }
  return std::string{ std::istreambuf_iterator<char>{ in }, std::istreambuf_iterator<char>{} };
}

inline std::optional<std::vector<char>> GetBinFromFile(const std::string& filename)
{
  std::ifstream in(filename, std::ios::binary);
  if (in.is_open() == false)
  {
    return std::nullopt;
  }
  return std::vector<char>{ std::istreambuf_iterator<char>{ in },
                            std::istreambuf_iterator<char>{} };
}

class IReader : public std::enable_shared_from_this<IReader>
{
public:
  virtual nlohmann::json readFormat() = 0;
  virtual std::map<std::string, std::vector<char>> readResource() = 0;
};

} // namespace VGG
