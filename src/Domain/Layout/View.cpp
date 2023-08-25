#include "View.hpp"

#include "AutoLayout.hpp"

namespace VGG
{

std::shared_ptr<Layout::Internal::AutoLayout> LayoutView::resetAutoLayout()
{
  m_autoLayout.reset(new Layout::Internal::AutoLayout);
  m_autoLayout->view = weak_from_this();

  return m_autoLayout;
}

std::shared_ptr<flexbox_node> LayoutView::createFlexboxNode()
{
  flexbox_node::p_node node{ new flexbox_node };
  m_autoLayout->flexNode = node;

  return m_autoLayout->flexNode;
}

std::shared_ptr<grid_layout> LayoutView::createGridNode()
{
  // todo
  return m_autoLayout->gridNode;
}

void LayoutView::applyLayout()
{
  for (auto subview : m_children)
  {
    subview->applyLayout();
  }

  if (m_autoLayout)
  {
    m_autoLayout->applyLayout(false);
  }
}

} // namespace VGG