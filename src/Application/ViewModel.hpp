#pragma once

#include "Domain/Daruma.hpp"
#include "Domain/Layout/Node.hpp"

namespace VGG
{
namespace Layout
{
class Layout;
}

struct ViewModel
{
  std::weak_ptr<Daruma> model;
  std::weak_ptr<Layout::Layout> layout;

  std::shared_ptr<LayoutNode> layoutTree() const;
  JsonDocumentPtr designDoc() const;

  VGG::Model::Loader::ResourcesType resources() const
  {
    if (auto sharedModel = model.lock())
    {
      return sharedModel->resources();
    }
    else
    {
      return {};
    }
  }
};

} // namespace VGG