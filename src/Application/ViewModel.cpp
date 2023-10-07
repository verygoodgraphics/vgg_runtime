#include "ViewModel.hpp"

#include "Domain/Layout/Layout.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Utility/Log.hpp"

using namespace VGG;

std::shared_ptr<LayoutNode> ViewModel::layoutTree() const
{
  auto sharedLayout = layout.lock();
  ASSERT(sharedLayout);
  if (sharedLayout)
  {
    return sharedLayout->layoutTree();
  }

  return {};
}

JsonDocumentPtr ViewModel::designDoc() const
{
  auto sharedLayout = layout.lock();
  ASSERT(sharedLayout);
  if (sharedLayout)
  {
    return sharedLayout->displayDesignDoc();
  }

  return JsonDocumentPtr(new RawJsonDocument{});
}