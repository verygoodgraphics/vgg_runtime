#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/Node.hpp"

namespace VGG
{

struct ViewModel
{
  std::shared_ptr<Daruma> model;

  nlohmann::json designDoc;
  std::shared_ptr<LayoutNode> layoutTree;

  auto resources() const
  {
    return model->resources();
  }
};

} // namespace VGG