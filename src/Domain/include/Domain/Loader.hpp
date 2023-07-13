#pragma once

#include "Config.hpp"

#include <map>
#include <string>
#include <vector>

namespace VGG
{
namespace Model
{

class Loader
{
public:
  using ResourcesType = std::map<std::string, std::vector<char>>;

  virtual ~Loader() = default;
  virtual bool readFile(const std::string& name, std::string& content) const = 0;
  virtual ResourcesType resources() const = 0;
};

} // namespace Model
} // namespace VGG