#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/View.hpp"

namespace VGG
{

struct ViewModel
{
  std::shared_ptr<Daruma> model;

  nlohmann::json designDoc;
  std::shared_ptr<LayoutView> layoutTree;

  auto resources() const
  {
    return model->resources();
  }
};

} // namespace VGG