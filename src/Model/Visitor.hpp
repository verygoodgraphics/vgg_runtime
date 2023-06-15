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
  virtual void accept(const std::string& path, const std::string& content) = 0;
};

} // namespace Model
} // namespace VGG