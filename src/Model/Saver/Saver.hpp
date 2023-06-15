#pragma once

#include "Model/Visitor.hpp"

namespace VGG
{
namespace Model
{

class Saver : public Visitor
{
public:
  virtual ~Saver() = default;

  virtual void accept(const std::string& path, const std::string& content) override = 0;
};

} // namespace Model
} // namespace VGG