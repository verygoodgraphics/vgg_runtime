#pragma once

#include <string>

namespace VGG
{
namespace Model
{

class Loader
{
public:
  virtual ~Loader() = default;
  virtual bool readFile(const std::string& name, std::string& content) const = 0;
};

} // namespace Model
} // namespace VGG