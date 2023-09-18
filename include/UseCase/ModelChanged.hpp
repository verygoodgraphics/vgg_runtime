#pragma once

#include <memory>

namespace VGG
{
class Daruma;

class ModelChanged
{
public:
  void onChange(std::shared_ptr<Daruma> model);
};

} // namespace VGG