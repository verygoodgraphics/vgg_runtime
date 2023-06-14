#pragma once

namespace VGG
{
namespace Model
{

class Loader
{
public:
  virtual ~Loader() = default;
  virtual bool load() = 0;
  virtual bool readFile(const std::string& name, std::string& content) const = 0;
};

} // namespace Model
} // namespace VGG