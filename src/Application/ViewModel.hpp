#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/Node.hpp"

namespace VGG
{

struct ViewModel
{
  std::shared_ptr<Daruma> model;

  std::shared_ptr<LayoutNode> layoutTree;

  const nlohmann::json& designDoc() const
  {
    return model->runtimeDesignDoc()->content();
  }
  auto resources() const
  {
    return model->resources();
  }
};

} // namespace VGG