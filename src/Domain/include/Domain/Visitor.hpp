#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace VGG
{
namespace Model
{

class Visitor
{
public:
  void accept(const std::string& path, const std::string& content)
  {
    accept(path, std::vector<char>(content.begin(), content.end()));
  }

  virtual void accept(const std::string& path, const std::vector<char>& content) = 0;
};

} // namespace Model
} // namespace VGG